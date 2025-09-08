#pragma once
#include <atomic>
#include <thread>

class LiveMetricAggregator {
public:
    LiveMetricAggregator(std::atomic<bool>& shutdown_flag);
    ~LiveMetricAggregator();

    void start();
    void stop();

private:
    void run();
    std::atomic<bool>& shutdown_flag_;
    std::thread worker_;
    bool running_ = false;
};