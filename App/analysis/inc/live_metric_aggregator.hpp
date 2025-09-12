#pragma once
#include <atomic>
#include <thread>
#include <unordered_map>

class MonitorDashboard;

class LiveMetricAggregator {
public:
    LiveMetricAggregator(std::atomic<bool>& shutdown_flag, MonitorDashboard* dashboard, int ui_refresh_interval_ms);
    ~LiveMetricAggregator();

    void start();
    void stop();

private:
    void run();
    std::atomic<bool>& shutdown_flag_;
    MonitorDashboard* dashboard_;
    int ui_refresh_interval_ms_;
    std::thread worker_;
    bool running_ = false;
    std::unordered_map<std::string, int64_t> last_update_map_;
};