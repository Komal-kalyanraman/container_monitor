#pragma once
#include <map>
#include <mutex>
#include <string>
#include <atomic>
#include <thread>
#include <condition_variable>
#include "common.hpp"

class MonitorDashboard {
public:
    MonitorDashboard(std::atomic<bool>& shutdown_flag, const MonitorConfig& cfg);
    ~MonitorDashboard();

    void pushMetrics(const ContainerMaxMetricsMsg& metrics);
    void pushMetricsRemoved(const std::string& container_id);

    void start();
    void stop();

private:
    void run();

    std::atomic<bool>& shutdown_flag_;
    const MonitorConfig& cfg_;
    std::thread worker_;
    bool running_ = false;

    std::mutex data_mutex_;
    std::condition_variable data_cv_;
    std::map<std::string, std::pair<ContainerMaxMetricsMsg, int64_t>> metrics_map_;
    bool data_updated_ = false;
    bool printed_empty_ = false;
};