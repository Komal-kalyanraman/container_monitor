#include "monitor_dashboard.hpp"
#include <ncurses.h>
#include <chrono>
#include <iostream>
#include "logger.hpp"
#include "common.hpp"

MonitorDashboard::MonitorDashboard(std::atomic<bool>& shutdown_flag, const MonitorConfig& cfg)
    : shutdown_flag_(shutdown_flag), cfg_(cfg) {
    CM_LOG_INFO << "[MonitorDashboard] ui_refresh_interval_ms_: " << cfg_.ui_refresh_interval_ms << "\n";
}

MonitorDashboard::~MonitorDashboard() {
    stop();
}

void MonitorDashboard::pushMetrics(const ContainerMaxMetricsMsg& metrics) {
    int64_t now = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    {
        std::lock_guard<std::mutex> lock(data_mutex_);
        CM_LOG_INFO << "[MonitorDashboard] pushMetrics: " << metrics.container_id
                    << " | CPU: " << metrics.max_cpu_usage_percent
                    << " | Mem: " << metrics.max_memory_usage_percent
                    << " | PIDs: " << metrics.max_pids_percent << "\n";
        metrics_map_[metrics.container_id] = std::make_pair(metrics, now);
        data_updated_ = true;
    }
    data_cv_.notify_one();
}

void MonitorDashboard::pushMetricsRemoved(const std::string& container_id) {
    {
        std::lock_guard<std::mutex> lock(data_mutex_);
        if (metrics_map_.find(container_id) != metrics_map_.end()) {
            metrics_map_.erase(container_id);
            CM_LOG_INFO << "[MonitorDashboard] pushMetricsRemoved: " << container_id << "\n";
            data_updated_ = true;
        } else {
            CM_LOG_INFO << "[MonitorDashboard] pushMetricsRemoved: container_id not found: " << container_id << "\n";
            data_updated_ = false;
        }
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
    initscr();
    cbreak();
    noecho();
    curs_set(0);

    // Initialize color support
    if (has_colors()) {
        start_color();
        init_pair(1, COLOR_GREEN, COLOR_BLACK);  // Safe
        init_pair(2, COLOR_YELLOW, COLOR_BLACK); // Warning
        init_pair(3, COLOR_RED, COLOR_BLACK);    // Critical
    }

    while (running_ && !shutdown_flag_) {
        std::unique_lock<std::mutex> lock(data_mutex_);
        data_cv_.wait_for(lock, std::chrono::milliseconds(cfg_.ui_refresh_interval_ms), [this] { return data_updated_ || !running_ || shutdown_flag_; });

        if (!running_ || shutdown_flag_) break;

        size_t max_name_len = std::string(COL_CONTAINER_NAME).length();
        for (const auto& [id, _] : metrics_map_) {
            if (id.length() > max_name_len) max_name_len = id.length();
        }
        max_name_len += 2;  // Added some padding

        std::string header_fmt = "%-" + std::to_string(max_name_len) + "s | %10s | %13s | %10s";
        std::string row_fmt    = "%-" + std::to_string(max_name_len) + "s | %10.2f | %13.2f | %10.2f";

        clear();
        mvprintw(0, 0, header_fmt.c_str(), COL_CONTAINER_NAME, COL_MAX_CPU, COL_MAX_MEM, COL_MAX_PIDS);
        int row = 1;

        if (metrics_map_.empty()) {
            mvprintw(row++, 0, "No containers to display.");
        } else {
            for (const auto& [id, pair] : metrics_map_) {
                const auto& metrics = pair.first;

                // Determine color for each metric
                auto color_for = [&](double value) {
                    if (value <= cfg_.alert_warning) return 1; // green
                    if (value <= cfg_.alert_critical) return 2; // yellow
                    return 3; // red
                };

                // Print container name (default color)
                mvprintw(row, 0, ("%-" + std::to_string(max_name_len) + "s |").c_str(), id.c_str());

                // Print CPU usage with color
                attron(COLOR_PAIR(color_for(metrics.max_cpu_usage_percent)));
                mvprintw(row, max_name_len + 3, "%10.2f", metrics.max_cpu_usage_percent);
                attroff(COLOR_PAIR(color_for(metrics.max_cpu_usage_percent)));

                // Print Memory usage with color
                attron(COLOR_PAIR(color_for(metrics.max_memory_usage_percent)));
                mvprintw(row, max_name_len + 16, "%13.2f", metrics.max_memory_usage_percent);
                attroff(COLOR_PAIR(color_for(metrics.max_memory_usage_percent)));

                // Print PIDs usage with color
                attron(COLOR_PAIR(color_for(metrics.max_pids_percent)));
                mvprintw(row, max_name_len + 30, "%10.2f", metrics.max_pids_percent);
                attroff(COLOR_PAIR(color_for(metrics.max_pids_percent)));

                row++;
            }
        }
        refresh();
        data_updated_ = false;
    }
    endwin();
}