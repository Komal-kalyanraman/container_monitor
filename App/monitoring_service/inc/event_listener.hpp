#pragma once

#include <thread>
#include <atomic>
#include <string>
#include "common.hpp"
#include "event_queue.hpp"

class RuntimeEventListener {
public:
    RuntimeEventListener(const MonitorConfig& config, EventQueue& queue, std::atomic<bool>& shutdown_flag);
    ~RuntimeEventListener();

    void start();
    void stop();

private:
    void eventThreadFunc();

    MonitorConfig config_;
    EventQueue& event_queue_;
    std::atomic<bool>& shutdown_flag_;
    std::thread event_thread_;
    std::atomic<bool> running_;
};