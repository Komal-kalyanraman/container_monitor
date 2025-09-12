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
#include "initializer.hpp"
#include "event_queue.hpp"
#include "config_parser.hpp"
#include "event_listener.hpp"
#include "event_processor.hpp"
#include "resource_monitor.hpp"
#include "sqlite_database.hpp"
#include "monitor_dashboard.hpp"
#include "resource_thread_pool.hpp"
#include "live_metric_aggregator.hpp"

std::atomic<bool> shutdown_requested{false};

// Signal handler to set the shutdown flag
void SignalHandler(int signum) {
    shutdown_requested = true;
    CM_LOG_INFO << "Shutdown signal received. Stopping all services... \n";
}

int main(int argc, char* argv[]) {
    // Parse configuration parameters
    MonitorConfig cfg = Initializer::parseConfig();
    
    // Initialize glog if needed
    Initializer::initLogger(argc, argv, cfg);
    
    // Ensure message queue is unlinked on startup
    Initializer::unlinkMessageQueue();
    
    // Setup signal handlers for graceful shutdown
    Initializer::setupSignalHandlers(SignalHandler);

    // Event queue for processing container events
    auto event_queue = std::make_shared<EventQueue>();
    
    // Vector to hold all worker threads
    std::vector<std::thread> worker_threads;

    // Initialize database interface
    SQLiteDatabase db(cfg.db_path); 
    
    // Clear existing data and setup schema
    db.clearAll();
    db.setupSchema();

    // Initialize resource thread pool
    ResourceThreadPool thread_pool(cfg, shutdown_requested, db);
    thread_pool.start();

    // Create worker objects as unique_ptr
    auto event_listener = std::make_unique<RuntimeEventListener>(cfg, *event_queue, shutdown_requested);
    auto event_processor = std::make_unique<EventProcessor>(*event_queue, shutdown_requested, db, cfg);
    auto resource_monitor = std::make_unique<ResourceMonitor>(db, shutdown_requested, thread_pool);

    // Create UI components
    std::unique_ptr<MonitorDashboard> monitor_dashboard;
    std::unique_ptr<LiveMetricAggregator> live_metric_aggregator;
    
    // Only initialize if UI is enabled in config
    if (cfg.ui_enabled) {
        monitor_dashboard = std::make_unique<MonitorDashboard>(shutdown_requested, cfg);
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

    // Start live metric aggregator if UI is enabled
    if (cfg.ui_enabled && live_metric_aggregator) {
        worker_threads.emplace_back([&](){ live_metric_aggregator->start(); });
    }

    // Main thread waits for shutdown signal
    while (!shutdown_requested) {
        std::this_thread::sleep_for(std::chrono::milliseconds(MAIN_LOOP_SLEEP_MS));
    }

    // Shutdown thread pool first to stop resource collection
    thread_pool.stop();

    // Stop event listener and processor
    event_listener->stop();
    event_processor->stop();

    // Stop resource monitor
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

    // Export container metrics to a file before shutdown
    db.exportAllTablesToCSV(cfg.file_export_folder_path);
    CM_LOG_INFO << "Container metrics exported to CSV at: " << cfg.file_export_folder_path << "\n";
    CM_LOG_INFO << "Application shutdown complete.\n";
    
    // Release glog resources
    google::ShutdownGoogleLogging();

    return 0;
}