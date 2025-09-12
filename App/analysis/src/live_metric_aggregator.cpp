/**
 * @file live_metric_aggregator.cpp
 * @brief Implements the LiveMetricAggregator class for real-time container metrics aggregation.
 */

#include "live_metric_aggregator.hpp"
#include <iostream>
#include <mqueue.h>
#include <cstring>
#include <cerrno>
#include <thread>
#include <chrono>
#include "logger.hpp"
#include "common.hpp"
#include "monitor_dashboard.hpp"

/**
 * @brief Constructs a LiveMetricAggregator.
 * @param shutdown_flag Reference to the application's shutdown flag.
 * @param dashboard Pointer to the MonitorDashboard instance.
 * @param ui_refresh_interval_ms UI refresh interval in milliseconds.
 */
LiveMetricAggregator::LiveMetricAggregator(std::atomic<bool>& shutdown_flag, MonitorDashboard* dashboard, int ui_refresh_interval_ms)
    : shutdown_flag_(shutdown_flag), dashboard_(dashboard), ui_refresh_interval_ms_(ui_refresh_interval_ms) {}

/**
 * @brief Destructor. Ensures the worker thread is stopped.
 */
LiveMetricAggregator::~LiveMetricAggregator() {
    stop();
}

/**
 * @brief Starts the aggregator thread.
 */
void LiveMetricAggregator::start() {
    if (!running_) {
        running_ = true;
        worker_ = std::thread(&LiveMetricAggregator::run, this);
    }
}

/**
 * @brief Stops the aggregator thread.
 */
void LiveMetricAggregator::stop() {
    running_ = false;
    if (worker_.joinable()) {
        worker_.join();
    }
}

/**
 * @brief Worker thread function. Handles message queue reading and dashboard updates.
 *
 * - Waits for the POSIX message queue to appear.
 * - Reads messages containing container metrics.
 * - Updates the dashboard with new metrics.
 * - Periodically removes stale containers.
 * - Handles graceful shutdown and message queue cleanup.
 */
void LiveMetricAggregator::run() {
    CM_LOG_INFO << "Waiting for message queue '" << METRIC_MQ_NAME << "' to appear... \n";

    mqd_t mqd;
    int attempts = 0;
    while (attempts < 50 && !shutdown_flag_) {
        mqd = mq_open(METRIC_MQ_NAME.data(), O_RDONLY | O_NONBLOCK);
        if (mqd != (mqd_t)-1) {
            CM_LOG_INFO << "Message queue opened successfully on attempt " << (attempts + 1) << ". \n";
            break;
        } else {
            CM_LOG_WARN << "Attempt " << (attempts + 1) << ": Queue not found, retrying... \n";
            attempts++;
            std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_MS_LONG));
        }
    }
    if (mqd == (mqd_t)-1) {
        CM_LOG_FATAL << "Message queue not found after waiting. \n";
        return;
    }

    CM_LOG_INFO << "Waiting for messages... \n";
    ContainerMaxMetricsMsg msg;
    unsigned int prio;
    int64_t last_cleanup = 0;
    while (running_ && !shutdown_flag_) {
        ssize_t bytes = mq_receive(mqd, reinterpret_cast<char*>(&msg), METRIC_MQ_MSG_SIZE, &prio);
        int64_t now = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();

        if (bytes >= 0) {
            std::string id(msg.container_id);
            last_update_map_[id] = now;
            if (dashboard_) {
                dashboard_->pushMetrics(msg);
            }
        } else if (errno == EAGAIN) {
            // No message, just wait
        } else if (errno == EINTR) {
            break; // Interrupted by signal
        } else {
            CM_LOG_ERROR << "mq_receive error: " << strerror(errno) << "\n";
        }

        // Periodically check for stale containers
        if (now - last_cleanup > ui_refresh_interval_ms_) {
            last_cleanup = now;
            for (auto it = last_update_map_.begin(); it != last_update_map_.end(); ) {
                if (now - it->second > ui_refresh_interval_ms_) {
                    // CM_LOG_INFO << "[Aggregator] Removing stale container: " << it->first << "\n";
                    if (dashboard_) {
                        dashboard_->pushMetricsRemoved(it->first);
                    }
                    it = last_update_map_.erase(it);
                } else {
                    ++it;
                }
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_MS_SHORT));
    }
    mq_close(mqd);
    CM_LOG_INFO << "Message queue closed. \n";
}