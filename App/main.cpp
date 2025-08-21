#include <iostream>
#include <memory>
#include <thread>
#include <vector>
#include <atomic>
#include <csignal>
#include "logger.hpp"
#include "common.hpp"
#include "config_parser.hpp"
#include "event_queue.hpp"
#include "event_listener.hpp"
#include "event_processor.hpp"

std::atomic<bool> shutdown_requested{false};

void SignalHandler(int signum) {
    shutdown_requested = true;
    std::cout << "Shutdown signal received. Stopping all services..." << std::endl;
}

int main() {
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

    // Start event listener
    RuntimeEventListener event_listener(cfg, *event_queue, shutdown_requested);
    EventProcessor event_processor(*event_queue, shutdown_requested);
    
    // Start event listener
    worker_threads.emplace_back([&](){ event_listener.start(); });
    
    // Start event processor
    worker_threads.emplace_back([&](){ event_processor.start(); });

    // Main loop: wait for shutdown signal
    while (!shutdown_requested) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    event_listener.stop();
    event_processor.stop();

    // Join all threads
    for (auto& thread : worker_threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }

    CM_LOG_INFO << "Application shutdown complete.\n";
    return 0;
}