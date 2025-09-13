/**
 * @file monitor_dashboard.cpp
 * @brief Implements the MonitorDashboard class for ncurses-based container metrics UI.
 */

#include "monitor_dashboard.hpp"
#include <ncurses.h>
#include <chrono>
#include <iostream>
#include <algorithm>
#include "logger.hpp"
#include "common.hpp"

/**
 * @brief Constructs a MonitorDashboard.
 * @param shutdown_flag Reference to the application's shutdown flag.
 * @param cfg Reference to monitor configuration.
 */
MonitorDashboard::MonitorDashboard(std::atomic<bool>& shutdown_flag, const MonitorConfig& cfg)
    : shutdown_flag_(shutdown_flag), cfg_(cfg) {
    CM_LOG_INFO << "[MonitorDashboard] ui_refresh_interval_ms_: " << cfg_.ui_refresh_interval_ms << "\n";
}

/**
 * @brief Destructor. Ensures the UI thread is stopped.
 */
MonitorDashboard::~MonitorDashboard() {
    stop();
}

/**
 * @brief Pushes new metrics for a container.
 * @param metrics ContainerMaxMetricsMsg struct.
 */
void MonitorDashboard::pushMetrics(const ContainerMaxMetricsMsg& metrics) {
    int64_t now = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    {
        std::lock_guard<std::mutex> lock(data_mutex_);
        CM_LOG_INFO << "[MonitorDashboard] pushMetrics: " << metrics.container_id
                    << " | CPU: " << metrics.max_cpu_usage_percent
                    << " | Mem: " << metrics.max_memory_usage_percent
                    << " | PIDs: " << metrics.max_pids_percent << "\n";
        // Check if container already exists, update if found, else append
        bool found = false;
        for (auto& entry : metrics_vec_) {
            if (entry.container_id == metrics.container_id) {
                entry.metrics = metrics;
                entry.timestamp = now;
                found = true;
                break;
            }
        }
        if (!found) {
            metrics_vec_.push_back({metrics.container_id, metrics, now});
        }
        data_updated_ = true;
    }
    data_cv_.notify_one();
}

/**
 * @brief Removes metrics for a container.
 * @param container_id Container identifier.
 */
void MonitorDashboard::pushMetricsRemoved(const std::string& container_id) {
    {
        std::lock_guard<std::mutex> lock(data_mutex_);
        auto it = std::remove_if(metrics_vec_.begin(), metrics_vec_.end(),
                                 [&](const ContainerMetricsEntry& entry) {
                                     return entry.container_id == container_id;
                                 });
        if (it != metrics_vec_.end()) {
            metrics_vec_.erase(it, metrics_vec_.end());
            CM_LOG_INFO << "[MonitorDashboard] pushMetricsRemoved: " << container_id << "\n";
            data_updated_ = true;
        } else {
            CM_LOG_INFO << "[MonitorDashboard] pushMetricsRemoved: container_id not found: " << container_id << "\n";
            data_updated_ = false;
        }
    }
    data_cv_.notify_one();
}

/**
 * @brief Starts the dashboard UI thread.
 */
void MonitorDashboard::start() {
    if (!running_) {
        running_ = true;
        worker_ = std::thread(&MonitorDashboard::run, this);
    }
}

/**
 * @brief Stops the dashboard UI thread.
 */
void MonitorDashboard::stop() {
    running_ = false;
    data_cv_.notify_one();
    if (worker_.joinable()) {
        worker_.join();
    }
}

/**
 * @brief Worker thread function. Handles ncurses UI rendering and updates.
 *
 * - Initializes ncurses and color pairs.
 * - Waits for metric updates or refresh interval.
 * - Dynamically aligns columns based on container name length.
 * - Displays metrics with color coding for alert thresholds.
 * - Handles shutdown and cleans up ncurses.
 */
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
        for (const auto& entry : metrics_vec_) {
            if (entry.container_id.length() > max_name_len) max_name_len = entry.container_id.length();
        }
        max_name_len += 2;  // Added some padding

        std::string header_fmt = "%-" + std::to_string(max_name_len) + "s | %10s | %13s | %10s";
        std::string row_fmt    = "%-" + std::to_string(max_name_len) + "s | %10.2f | %13.2f | %10.2f";

        clear();
        mvprintw(0, 0, header_fmt.c_str(), COL_CONTAINER_NAME, COL_MAX_CPU, COL_MAX_MEM, COL_MAX_PIDS);
        int row = 1;

        if (metrics_vec_.empty()) {
            mvprintw(row++, 0, "No containers to display.");
        } else {
            for (const auto& entry : metrics_vec_) {
                const auto& metrics = entry.metrics;
                const auto& id = entry.container_id;

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