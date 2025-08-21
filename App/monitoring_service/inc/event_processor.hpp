#pragma once
#include "event_queue.hpp"
#include <atomic>
#include <thread>

class EventProcessor {
public:
    EventProcessor(EventQueue& queue, std::atomic<bool>& shutdown_flag);
    ~EventProcessor();
    void start();
    void stop();

private:
    void processLoop();
    EventQueue& queue_;
    std::atomic<bool>& shutdown_flag_;
    std::thread worker_;
    bool running_ = false;
};