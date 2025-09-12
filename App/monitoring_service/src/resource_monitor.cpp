#include "resource_monitor.hpp"
#include "resource_thread_pool.hpp"
#include "logger.hpp"
#include <thread>
#include <chrono>
#include <set>

ResourceMonitor::ResourceMonitor(IDatabaseInterface& db, std::atomic<bool>& shutdown_flag, ResourceThreadPool& thread_pool)
    : db_(db), shutdown_flag_(shutdown_flag), thread_pool_(thread_pool) {}

void ResourceMonitor::start() {
    running_ = true;
    monitor_thread_ = std::thread(&ResourceMonitor::monitorLoop, this);
}

void ResourceMonitor::stop() {
    running_ = false;
    if (monitor_thread_.joinable()) monitor_thread_.join();
}

void ResourceMonitor::monitorLoop() {
    std::set<std::string> previous_containers;
    while (running_ && !shutdown_flag_) {
        std::set<std::string> current_containers;
        for (const auto& entry : db_.getAll()) {
            current_containers.insert(entry.first);
        }

        // Detect new containers (create event)
        for (const auto& name : current_containers) {
            if (previous_containers.find(name) == previous_containers.end()) {
                thread_pool_.addContainer(name);
                CM_LOG_INFO << "[ResourceMonitor] Detected new container: " << name << "\n";
            }
        }

        // Detect removed containers (destroy event)
        for (const auto& name : previous_containers) {
            if (current_containers.find(name) == current_containers.end()) {
                thread_pool_.removeContainer(name);
                CM_LOG_INFO << "[ResourceMonitor] Detected removed container: " << name << "\n";
            }
        }

        previous_containers = std::move(current_containers);

        std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_MS_LONG));
    }
}