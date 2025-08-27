#include "resource_monitor.hpp"
#include <iostream>
#include <thread>
#include <chrono>

ResourceMonitor::ResourceMonitor(IDatabaseInterface& db, std::atomic<bool>& shutdown_flag)
    : db_(db), shutdown_flag_(shutdown_flag) {}

void ResourceMonitor::start() {
    running_ = true;
    monitor_thread_ = std::thread(&ResourceMonitor::monitorLoop, this);
}

void ResourceMonitor::stop() {
    running_ = false;
    if (monitor_thread_.joinable()) monitor_thread_.join();
}

void ResourceMonitor::monitorLoop() {
    while (running_ && !shutdown_flag_) {
        if (db_.size() > 0) {
            std::cout << "[ResourceMonitor] Current database entries:\n";
            for (const auto& entry : db_.getAll()) {
                const auto& name = entry.first;
                const auto& tup = entry.second;
                std::cout << "Name: " << name
                          << ", ID: " << std::get<0>(tup)
                          << ", CPUs: " << std::get<1>(tup)
                          << ", Memory: " << std::get<2>(tup)
                          << ", PIDs limit: " << std::get<3>(tup)
                          << std::endl;
            }
        }
        std::this_thread::sleep_for(std::chrono::seconds(2));
    }
}