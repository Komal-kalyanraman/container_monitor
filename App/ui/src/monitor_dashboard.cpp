#include "monitor_dashboard.hpp"
#include <ncurses.h>
#include <chrono>
#include <iostream>

MonitorDashboard::MonitorDashboard(std::atomic<bool>& shutdown_flag, int ui_refresh_interval_ms)
    : shutdown_flag_(shutdown_flag), ui_refresh_interval_ms_(ui_refresh_interval_ms) {
    std::cout << "[MonitorDashboard] ui_refresh_interval_ms_: " << ui_refresh_interval_ms_ << std::endl;
}

MonitorDashboard::~MonitorDashboard() {
    stop();
}

void MonitorDashboard::pushMetrics(const ContainerMaxMetricsMsg& metrics) {
    int64_t now = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    {
        std::lock_guard<std::mutex> lock(data_mutex_);
        std::cout << "[MonitorDashboard] pushMetrics: " << metrics.container_id
                  << " | CPU: " << metrics.max_cpu_usage_percent
                  << " | Mem: " << metrics.max_memory_usage_percent
                  << " | PIDs: " << metrics.max_pids_percent << std::endl;
        metrics_map_[metrics.container_id] = std::make_pair(metrics, now);
        data_updated_ = true;
    }
    data_cv_.notify_one();
}

void MonitorDashboard::pushMetricsRemoved(const std::string& container_id) {
    {
        std::lock_guard<std::mutex> lock(data_mutex_);
        std::cout << "[MonitorDashboard] pushMetricsRemoved: " << container_id << std::endl;
        metrics_map_.erase(container_id);
        data_updated_ = true;
    }
    data_cv_.notify_one();
}

void MonitorDashboard::start() {
    if (!running_) {
        running_ = true;
        worker_ = std::thread(&MonitorDashboard::run, this);
    }
}

void MonitorDashboard::stop() {
    running_ = false;
    data_cv_.notify_one();
    if (worker_.joinable()) {
        worker_.join();
    }
}

void MonitorDashboard::run() {
    // initscr();
    // cbreak();
    // noecho();
    // curs_set(0);

    while (running_ && !shutdown_flag_) {
        std::unique_lock<std::mutex> lock(data_mutex_);
        // std::cout << "[MonitorDashboard] Waiting for data_cv_..." << std::endl;
        data_cv_.wait_for(lock, std::chrono::milliseconds(ui_refresh_interval_ms_), [this] { return data_updated_ || !running_ || shutdown_flag_; });

        if (!running_ || shutdown_flag_) break;

        // Print metrics_map_ for debugging
        if (metrics_map_.empty()) {
            if (!printed_empty_) {
                std::cout << "[MonitorDashboard] No containers to display." << std::endl;
                printed_empty_ = true;
            }
        } else {
            printed_empty_ = false;
            std::cout << "[MonitorDashboard] metrics_map_ dump:" << std::endl;
            for (const auto& [id, pair] : metrics_map_) {
                const auto& metrics = pair.first;
                std::cout << "  " << id
                        << " | CPU: " << metrics.max_cpu_usage_percent
                        << " | Mem: " << metrics.max_memory_usage_percent
                        << " | PIDs: " << metrics.max_pids_percent
                        << " | LastUpdate: " << pair.second << std::endl;
            }
        }

        // Remove stale containers (optional, for extra safety)
        int64_t now = std::chrono::duration_cast<std::chrono::milliseconds>(
                      std::chrono::system_clock::now().time_since_epoch()).count();
        for (auto it = metrics_map_.begin(); it != metrics_map_.end(); ) {
            if (now - it->second.second > ui_refresh_interval_ms_) {
                std::cout << "[MonitorDashboard] Removing stale container: " << it->first << std::endl;
                it = metrics_map_.erase(it);
                data_updated_ = true;
            } else {
                ++it;
            }
        }

        // Clear screen and print table header
        // clear();
        // mvprintw(0, 0, "Container Name | Max CPU %% | Max Memory %% | Max PIDs %%");
        // int row = 1;
        // Print metrics_map_ for debugging
        if (metrics_map_.empty()) {
            static bool printed_empty = false;
            if (!printed_empty) {
                std::cout << "[MonitorDashboard] No containers to display." << std::endl;
                printed_empty = true;
            }
        } else {
            static bool printed_empty = false;
            printed_empty = false;
            std::cout << "[MonitorDashboard] metrics_map_ dump:" << std::endl;
            for (const auto& [id, pair] : metrics_map_) {
                const auto& metrics = pair.first;
                std::cout << "  " << id
                        << " | CPU: " << metrics.max_cpu_usage_percent
                        << " | Mem: " << metrics.max_memory_usage_percent
                        << " | PIDs: " << metrics.max_pids_percent
                        << " | LastUpdate: " << pair.second << std::endl;
            }
        }
        // refresh();
        data_updated_ = false;
    }

    endwin();
}