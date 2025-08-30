#include "event_processor.hpp"
#include <iostream>
#include "logger.hpp"
#include "json_processing.hpp"
#include "metrics_reader.hpp"

EventProcessor::EventProcessor(EventQueue& queue, std::atomic<bool>& shutdown_flag, IDatabaseInterface& db, const MonitorConfig& cfg)
    : queue_(queue), shutdown_flag_(shutdown_flag), db_(db), cfg_(cfg) {}

EventProcessor::~EventProcessor() { stop(); }

void EventProcessor::start() {
    running_ = true;
    worker_ = std::thread(&EventProcessor::processLoop, this);
}

void EventProcessor::stop() {
    running_ = false;
    queue_.shutdown();
    if (worker_.joinable()) worker_.join();
}

void EventProcessor::processLoop() {
    std::string event;
    int refresh_interval = cfg_.container_event_refresh_interval_ms;
    HostInfo host_info = MetricsReader::getHostInfo();
    CM_LOG_INFO << "[Host Info] CPUs: " << host_info.num_cpus
                << ", Total Memory: " << host_info.total_memory_mb << " MB\n";
    // db_.saveHostInfo(host_info);

    while (running_ && !shutdown_flag_) {
        // Host usage collection
        auto now = std::chrono::system_clock::now();
        auto timestamp_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
    
        double cpu_usage = MetricsReader::getHostCpuUsage();
        uint64_t mem_usage_mb = MetricsReader::getHostMemoryUsageMB();
        db_.saveHostUsage(timestamp_ms, cpu_usage, mem_usage_mb);
        CM_LOG_INFO << "[Host Usage] Timestamp: " << timestamp_ms
              << ", CPU: " << cpu_usage << "%, Memory: " << mem_usage_mb << " MB\n";

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