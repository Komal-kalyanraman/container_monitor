#pragma once
#include <queue>
#include <mutex>
#include <condition_variable>
#include <string>

class EventQueue {
public:
    void push(const std::string& event);
    bool pop(std::string& event, int timeout_ms);
    void shutdown();

private:
    std::queue<std::string> queue_;
    std::mutex mutex_;
    std::condition_variable cv_;
    bool stopped_ = false;
};