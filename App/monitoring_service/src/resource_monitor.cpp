/**
 * @file resource_monitor.cpp
 * @brief Implements the ResourceMonitor class for tracking live containers and managing resource threads.
 */

#include "resource_monitor.hpp"
#include <set>
#include <thread>
#include <chrono>
#include "logger.hpp"
#include "resource_thread_pool.hpp"

/**
 * @brief Constructs a ResourceMonitor.
 * @param db Reference to the database interface.
 * @param shutdown_flag Reference to the application's shutdown flag.
 * @param thread_pool Reference to the resource thread pool.
 */
ResourceMonitor::ResourceMonitor(IDatabaseInterface& db, std::atomic<bool>& shutdown_flag, ResourceThreadPool& thread_pool)
    : db_(db), shutdown_flag_(shutdown_flag), thread_pool_(thread_pool) {}

/**
 * @brief Starts the resource monitor thread.
 */
void ResourceMonitor::start() {
    running_ = true;
    monitor_thread_ = std::thread(&ResourceMonitor::monitorLoop, this);
}

/**
 * @brief Stops the resource monitor thread.
 */
void ResourceMonitor::stop() {
    running_ = false;
    if (monitor_thread_.joinable()) monitor_thread_.join();
}

/**
 * @brief Worker thread function. Monitors container changes and updates the thread pool.
 *
 * - Periodically checks the database for current containers.
 * - Detects new containers and adds them to the thread pool.
 * - Detects removed containers and removes them from the thread pool.
 * - Sleeps between checks and handles shutdown.
 */
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