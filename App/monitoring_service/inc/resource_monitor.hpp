#pragma once
#include "database_interface.hpp"
#include <atomic>
#include <thread>

class ResourceMonitor {
public:
    ResourceMonitor(IDatabaseInterface& db, std::atomic<bool>& shutdown_flag);
    void start();
    void stop();
private:
    void monitorLoop();
    IDatabaseInterface& db_;
    std::atomic<bool>& shutdown_flag_;
    std::thread monitor_thread_;
    bool running_ = false;
};