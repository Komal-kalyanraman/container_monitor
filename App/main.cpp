#include <iostream>
#include <memory>
#include <thread>
#include <vector>
#include <atomic>
#include <csignal>
#include <cerrno>
#include <mqueue.h>
#include "logger.hpp"
#include "common.hpp"
#include "config_parser.hpp"
#include "event_queue.hpp"
#include "event_listener.hpp"
#include "event_processor.hpp"
#include "resource_monitor.hpp"
#include "sqlite_database.hpp"
#include "monitor_dashboard.hpp"
#include "resource_thread_pool.hpp"
#include "live_metric_aggregator.hpp"

std::atomic<bool> shutdown_requested{false};

void SignalHandler(int signum) {
    shutdown_requested = true;
    CM_LOG_INFO << "Shutdown signal received. Stopping all services... \n";
}

int main() {
    // Remove existing message queue at startup
    std::cout << "[Main] Attempting to unlink message queue: /container_max_metric_mq" << std::endl;
    int unlink_result = mq_unlink(METRIC_MQ_NAME.data());
    if (unlink_result == 0) {
        std::cout << "[Main] Successfully unlinked /container_max_metric_mq" << std::endl;
    } else {
        std::cout << "[Main] mq_unlink failed: " << strerror(errno) << std::endl;
    }

    std::signal(SIGINT, SignalHandler);
    std::signal(SIGTERM, SignalHandler);

    ConfigParser parser;
    if (!parser.load(CONFIG_FILE_PATH)) {
        CM_LOG_ERROR << "Failed to load configuration file. Exiting application\n";
        return 1;
    }
    MonitorConfig cfg = parser.toMonitorConfig();
    parser.printConfig(cfg);

    auto event_queue = std::make_shared<EventQueue>(); // Event queue for processing container events
    std::vector<std::thread> worker_threads;    // Worker threads for event processing
    
    SQLiteDatabase db(cfg.db_path); // Use path from config
    db.clearAll(); // Clear all entries at startup
    db.setupSchema(); // Initialize the container_metrics table

    ResourceThreadPool thread_pool(cfg, shutdown_requested, db);
    thread_pool.start();

    // Create worker objects as unique_ptr
    auto event_listener = std::make_unique<RuntimeEventListener>(cfg, *event_queue, shutdown_requested);
    auto event_processor = std::make_unique<EventProcessor>(*event_queue, shutdown_requested, db, cfg);
    auto resource_monitor = std::make_unique<ResourceMonitor>(db, shutdown_requested, thread_pool);

    std::unique_ptr<MonitorDashboard> monitor_dashboard;
    std::unique_ptr<LiveMetricAggregator> live_metric_aggregator;
    if (cfg.ui_enabled) {
        monitor_dashboard = std::make_unique<MonitorDashboard>(shutdown_requested, cfg.ui_refresh_interval_ms);
        live_metric_aggregator = std::make_unique<LiveMetricAggregator>(shutdown_requested, monitor_dashboard.get(), cfg.ui_refresh_interval_ms);
    }

    // Start event listener
    worker_threads.emplace_back([&](){ event_listener->start(); });    
    // Start event processor
    worker_threads.emplace_back([&](){ event_processor->start(); });
    // Start resource monitor thread
    worker_threads.emplace_back([&](){ resource_monitor->start(); });
    // Start UI components if enabled
    if (cfg.ui_enabled && monitor_dashboard) {
        worker_threads.emplace_back([&](){ monitor_dashboard->start(); });
    }
    if (cfg.ui_enabled && live_metric_aggregator) {
        worker_threads.emplace_back([&](){ live_metric_aggregator->start(); });
    }

    // Main loop: wait for shutdown signal
    while (!shutdown_requested) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    thread_pool.stop();

    event_listener->stop();
    event_processor->stop();
    resource_monitor->stop();
    // Stop UI components if running
    if (cfg.ui_enabled && live_metric_aggregator) {
        live_metric_aggregator->stop();
    }
    if (cfg.ui_enabled && monitor_dashboard) {
        monitor_dashboard->stop();
    }

    // Join all threads
    for (auto& thread : worker_threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }

    // Export container metrics to CSV before shutdown
    db.exportAllTablesToCSV(cfg.csv_export_folder_path);
    CM_LOG_INFO << "Container metrics exported to CSV at: " << cfg.csv_export_folder_path << "\n";

    CM_LOG_INFO << "Application shutdown complete.\n";
    return 0;
}