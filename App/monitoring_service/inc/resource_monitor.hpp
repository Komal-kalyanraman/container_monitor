#pragma once
#include "database_interface.hpp"
#include "resource_thread_pool.hpp"
#include <atomic>
#include <thread>

class ResourceMonitor {
public:
    ResourceMonitor(IDatabaseInterface& db, std::atomic<bool>& shutdown_flag, ResourceThreadPool& thread_pool);
    void start();
    void stop();
private:
    void monitorLoop();
    IDatabaseInterface& db_;
    std::atomic<bool>& shutdown_flag_;
    ResourceThreadPool& thread_pool_;
    std::thread monitor_thread_;
    bool running_ = false;
};