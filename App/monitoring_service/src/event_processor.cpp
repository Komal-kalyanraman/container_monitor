/**
 * @file event_processor.cpp
 * @brief Implements the EventProcessor class for processing container events and host metrics.
 */

#include "event_processor.hpp"
#include <iostream>
#include "logger.hpp"
#include "json_processing.hpp"
#include "metrics_reader.hpp"

/**
 * @brief Constructs an EventProcessor.
 * @param queue Reference to the event queue.
 * @param shutdown_flag Reference to the application's shutdown flag.
 * @param db Reference to the database interface.
 * @param cfg Reference to the monitor configuration.
 */
EventProcessor::EventProcessor(EventQueue& queue, std::atomic<bool>& shutdown_flag, IDatabaseInterface& db, const MonitorConfig& cfg)
    : queue_(queue), shutdown_flag_(shutdown_flag), db_(db), cfg_(cfg) {}

/**
 * @brief Destructor. Ensures the worker thread is stopped.
 */
EventProcessor::~EventProcessor() { stop(); }

/**
 * @brief Starts the event processor thread.
 */
void EventProcessor::start() {
    running_ = true;
    worker_ = std::thread(&EventProcessor::processLoop, this);
}

/**
 * @brief Stops the event processor thread.
 */
void EventProcessor::stop() {
    running_ = false;
    queue_.shutdown();
    if (worker_.joinable()) worker_.join();
}

/**
 * @brief Worker thread function. Processes events and collects host metrics.
 *
 * - Periodically collects host CPU and memory usage and saves to the database.
 * - Pops container events from the event queue, parses them, and updates the database.
 * - Handles container creation and destruction events.
 * - Handles shutdown and cleans up resources.
 */
void EventProcessor::processLoop() {
    std::string event;
    int refresh_interval = cfg_.container_event_refresh_interval_ms;
    MetricsReader metrics_reader({}, 0);
    HostInfo host_info = metrics_reader.getHostInfo();
    CM_LOG_INFO << "[Host Info] CPUs: " << host_info.num_cpus
                << ", Total Memory: " << host_info.total_memory_mb << " MB\n";
    // db_.saveHostInfo(host_info);

    while (running_ && !shutdown_flag_) {
        // Host usage collection
        auto now = std::chrono::system_clock::now();
        auto timestamp_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
    
        double cpu_usage_percentage = metrics_reader.getHostCpuUsagePercentage();
        double mem_usage_percentage = metrics_reader.getHostMemoryUsagePercent();
        db_.saveHostUsage(timestamp_ms, cpu_usage_percentage, mem_usage_percentage);

        if ((queue_.pop(event, refresh_interval))) {
            try {
                ContainerEventInfo info;
                if (parseContainerEvent(event, info)) {
                    CM_LOG_INFO << "[Container Event] "
                                << "Name: " << info.name
                                << ", ID: " << info.id
                                << ", Status: " << info.status
                                << ", Time (ns): " << info.timeNano;
                    if (info.status == "create") {
                        CM_LOG_INFO << ", CPUs: " << info.cpus
                                    << ", Memory: " << info.memory
                                    << ", PIDs limit: " << info.pids_limit;
                        double cpus = std::stod(info.cpus);
                        int memory = std::stoi(info.memory);
                        int pids_limit = std::stoi(info.pids_limit);
                        db_.saveContainer(info.name, ContainerInfo{info.id, cpus, memory, pids_limit});
                    } else if (info.status == "destroy") {
                        db_.removeContainer(info.name);
                        CM_LOG_INFO << " [Container Removed]";
                    }
                    CM_LOG_INFO << "\n";
                }
            } catch (const std::exception& e) {
                CM_LOG_ERROR << "Event processing error: " << e.what() << "\n";
            } catch (...) {
                CM_LOG_ERROR << "Unknown error during event processing. \n";
            }
        }        
    }
}