/**
 * @file event_listener.hpp
 * @brief Declares the RuntimeEventListener class for container event monitoring.
 */

#pragma once

#include <thread>
#include <atomic>
#include <string>
#include "common.hpp"
#include "event_queue.hpp"

/**
 * @class RuntimeEventListener
 * @brief Listens for container runtime events and pushes them to an event queue.
 *
 * Spawns a thread to execute the container runtime's event command (docker/podman),
 * reads events, and pushes them to the event queue for processing.
 */
class RuntimeEventListener {
public:
    /**
     * @brief Constructs a RuntimeEventListener.
     * @param config Monitor configuration.
     * @param queue Reference to the event queue.
     * @param shutdown_flag Reference to the application's shutdown flag.
     */
    RuntimeEventListener(const MonitorConfig& config, EventQueue& queue, std::atomic<bool>& shutdown_flag);

    /**
     * @brief Destructor. Ensures the event thread is stopped.
     */
    ~RuntimeEventListener();

    /**
     * @brief Starts the event listener thread.
     */
    void start();

    /**
     * @brief Stops the event listener thread.
     */
    void stop();

private:
    /**
     * @brief Worker thread function. Executes the event command and pushes events to the queue.
     */
    void eventThreadFunc();

    MonitorConfig config_;                ///< Monitor configuration.
    EventQueue& event_queue_;             ///< Reference to the event queue.
    std::atomic<bool>& shutdown_flag_;    ///< Reference to shutdown flag.
    std::thread event_thread_;            ///< Event listener thread.
    std::atomic<bool> running_;           ///< Indicates if the listener is running.
};