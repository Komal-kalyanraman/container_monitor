#pragma once

#include <thread>
#include <atomic>
#include <functional>
#include <string>
#include "common.hpp"

// Callback type for event notifications
using ContainerEventCallback = std::function<void(const std::string& event_json)>;

class RuntimeEventListener {
public:
    RuntimeEventListener(const MonitorConfig& config, ContainerEventCallback callback, std::atomic<bool>& shutdown_flag);
    ~RuntimeEventListener();

    void start();
    void stop();

private:
    void eventThreadFunc();

    MonitorConfig config_;
    ContainerEventCallback callback_;
    std::atomic<bool>& shutdown_flag_;
    std::thread event_thread_;
    std::atomic<bool> running_;
};