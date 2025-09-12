/**
 * @file monitor_dashboard.hpp
 * @brief Declares the MonitorDashboard class for ncurses-based container metrics UI.
 */

#pragma once
#include <map>
#include <mutex>
#include <string>
#include <atomic>
#include <thread>
#include <condition_variable>
#include "common.hpp"

/**
 * @class MonitorDashboard
 * @brief Displays live container metrics in a color-coded ncurses UI.
 *
 * Receives metrics via message queue, updates the display, and removes stale containers.
 * Supports dynamic column alignment and color coding based on alert thresholds.
 */
class MonitorDashboard {
public:
    /**
     * @brief Constructs a MonitorDashboard.
     * @param shutdown_flag Reference to the application's shutdown flag.
     * @param cfg Reference to monitor configuration.
     */
    MonitorDashboard(std::atomic<bool>& shutdown_flag, const MonitorConfig& cfg);

    /**
     * @brief Destructor. Ensures the UI thread is stopped.
     */
    ~MonitorDashboard();

    /**
     * @brief Pushes new metrics for a container.
     * @param metrics ContainerMaxMetricsMsg struct.
     */
    void pushMetrics(const ContainerMaxMetricsMsg& metrics);

    /**
     * @brief Removes metrics for a container.
     * @param container_id Container identifier.
     */
    void pushMetricsRemoved(const std::string& container_id);

    /**
     * @brief Starts the dashboard UI thread.
     */
    void start();

    /**
     * @brief Stops the dashboard UI thread.
     */
    void stop();

private:
    /**
     * @brief Worker thread function. Handles ncurses UI rendering and updates.
     */
    void run();

    std::atomic<bool>& shutdown_flag_;      ///< Reference to shutdown flag.
    const MonitorConfig& cfg_;              ///< Monitor configuration.
    std::thread worker_;                    ///< UI worker thread.
    bool running_ = false;                  ///< Indicates if the dashboard is running.

    std::mutex data_mutex_;                 ///< Mutex for metrics data.
    std::condition_variable data_cv_;       ///< Condition variable for UI updates.
    bool data_updated_ = false;             ///< Indicates if data was updated.
    bool printed_empty_ = false;            ///< Tracks if "No containers" message was printed.
    std::map<std::string, std::pair<ContainerMaxMetricsMsg, int64_t>> metrics_map_; ///< Container metrics map.
};