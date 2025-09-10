#pragma once
#include <string>
#include <map>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <thread>
#include "common.hpp"

class MonitorDashboard {
public:
    MonitorDashboard(std::atomic<bool>& shutdown_flag, int ui_refresh_interval_ms);
    ~MonitorDashboard();

    void pushMetrics(const ContainerMaxMetricsMsg& metrics);
    void pushMetricsRemoved(const std::string& container_id);

    void start();
    void stop();

private:
    void run();

    std::atomic<bool>& shutdown_flag_;
    int ui_refresh_interval_ms_;
    std::thread worker_;
    bool running_ = false;

    std::mutex data_mutex_;
    std::condition_variable data_cv_;
    // Map: container_id -> (metrics, last_update_time_ms)
    std::map<std::string, std::pair<ContainerMaxMetricsMsg, int64_t>> metrics_map_;
    bool data_updated_ = false;
    bool printed_empty_ = false;
};