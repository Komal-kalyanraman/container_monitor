/**
 * @file event_queue.cpp
 * @brief Implements the EventQueue class for thread-safe event management.
 */

#include "event_queue.hpp"

/**
 * @brief Pushes an event onto the queue and notifies one waiting thread.
 * @param event Event string to push.
 */
void EventQueue::push(const std::string& event) {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        queue_.push(event);
    }
    cv_.notify_one();
}

/**
 * @brief Pops an event from the queue, waiting up to timeout_ms milliseconds.
 * @param event Reference to string to store the popped event.
 * @param timeout_ms Timeout in milliseconds.
 * @return True if an event was popped, false on timeout or shutdown.
 */
bool EventQueue::pop(std::string& event, int timeout_ms) {
    std::unique_lock<std::mutex> lock(mutex_);
    if (!cv_.wait_for(lock, std::chrono::milliseconds(timeout_ms), [this]{ return !queue_.empty() || stopped_; })) {
        // Timeout occurred, no event
        return false;
    }
    if (stopped_ && queue_.empty()) return false;
    event = queue_.front();
    queue_.pop();
    return true;
}

/**
 * @brief Signals shutdown and wakes all waiting threads.
 */
void EventQueue::shutdown() {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        stopped_ = true;
    }
    cv_.notify_all();
}