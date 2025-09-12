/**
 * @file event_queue.hpp
 * @brief Declares the EventQueue class for thread-safe event management.
 */

#pragma once
#include <queue>
#include <mutex>
#include <condition_variable>
#include <string>

/**
 * @class EventQueue
 * @brief Thread-safe queue for container runtime events.
 *
 * Provides push and pop operations with timeout, and supports graceful shutdown.
 */
class EventQueue {
public:
    /**
     * @brief Pushes an event onto the queue.
     * @param event Event string to push.
     */
    void push(const std::string& event);

    /**
     * @brief Pops an event from the queue, waiting up to timeout_ms milliseconds.
     * @param event Reference to string to store the popped event.
     * @param timeout_ms Timeout in milliseconds.
     * @return True if an event was popped, false on timeout or shutdown.
     */
    bool pop(std::string& event, int timeout_ms);

    /**
     * @brief Signals shutdown and wakes all waiting threads.
     */
    void shutdown();

private:
    std::queue<std::string> queue_;           ///< Internal event queue.
    std::mutex mutex_;                        ///< Mutex for thread safety.
    std::condition_variable cv_;              ///< Condition variable for waiting.
    bool stopped_ = false;                    ///< Indicates if shutdown was requested.
};