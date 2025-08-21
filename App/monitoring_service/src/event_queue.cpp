#include "event_queue.hpp"

void EventQueue::push(const std::string& event) {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        queue_.push(event);
    }
    cv_.notify_one();
}

bool EventQueue::pop(std::string& event) {
    std::unique_lock<std::mutex> lock(mutex_);
    cv_.wait(lock, [this]{ return !queue_.empty() || stopped_; });
    if (stopped_ && queue_.empty()) return false;
    event = queue_.front();
    queue_.pop();
    return true;
}

void EventQueue::shutdown() {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        stopped_ = true;
    }
    cv_.notify_all();
}