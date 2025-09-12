/**
 * @file live_metric_aggregator.hpp
 * @brief Declares the LiveMetricAggregator class for real-time container metrics aggregation and dashboard updates.
 */

#pragma once
#include <atomic>
#include <thread>
#include <unordered_map>

class MonitorDashboard;

/**
 * @class LiveMetricAggregator
 * @brief Aggregates live container metrics from a POSIX message queue and updates the dashboard.
 *
 * Periodically reads messages containing container metrics, updates the dashboard,
 * and removes stale containers based on the UI refresh interval.
 */
class LiveMetricAggregator {
public:
    /**
     * @brief Constructs a LiveMetricAggregator.
     * @param shutdown_flag Reference to the application's shutdown flag.
     * @param dashboard Pointer to the MonitorDashboard instance.
     * @param ui_refresh_interval_ms UI refresh interval in milliseconds.
     */
    LiveMetricAggregator(std::atomic<bool>& shutdown_flag, MonitorDashboard* dashboard, int ui_refresh_interval_ms);

    /**
     * @brief Destructor. Ensures the worker thread is stopped.
     */
    ~LiveMetricAggregator();

    /**
     * @brief Starts the aggregator thread.
     */
    void start();

    /**
     * @brief Stops the aggregator thread.
     */
    void stop();

private:
    /**
     * @brief Worker thread function. Handles message queue reading and dashboard updates.
     */
    void run();

    std::atomic<bool>& shutdown_flag_;                ///< Reference to shutdown flag.
    MonitorDashboard* dashboard_;                     ///< Pointer to dashboard for metric updates.
    int ui_refresh_interval_ms_;                      ///< UI refresh interval in milliseconds.
    std::thread worker_;                              ///< Worker thread for aggregation.
    bool running_ = false;                            ///< Indicates if the aggregator is running.
    std::unordered_map<std::string, int64_t> last_update_map_; ///< Tracks last update time for each container.
};