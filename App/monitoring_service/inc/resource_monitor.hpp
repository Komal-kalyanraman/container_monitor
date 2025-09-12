/**
 * @file resource_monitor.hpp
 * @brief Declares the ResourceMonitor class for tracking live containers and managing resource threads.
 */

#pragma once
#include <atomic>
#include <thread>
#include "database_interface.hpp"
#include "resource_thread_pool.hpp"

/**
 * @class ResourceMonitor
 * @brief Monitors live containers and updates the resource thread pool accordingly.
 *
 * Periodically checks the database for container changes, detects new or removed containers,
 * and updates the resource thread pool to start or stop resource collection.
 */
class ResourceMonitor {
public:
    /**
     * @brief Constructs a ResourceMonitor.
     * @param db Reference to the database interface.
     * @param shutdown_flag Reference to the application's shutdown flag.
     * @param thread_pool Reference to the resource thread pool.
     */
    ResourceMonitor(IDatabaseInterface& db, std::atomic<bool>& shutdown_flag, ResourceThreadPool& thread_pool);

    /**
     * @brief Starts the resource monitor thread.
     */
    void start();

    /**
     * @brief Stops the resource monitor thread.
     */
    void stop();

private:
    /**
     * @brief Worker thread function. Monitors container changes and updates the thread pool.
     */
    void monitorLoop();

    IDatabaseInterface& db_;                ///< Reference to database interface.
    std::atomic<bool>& shutdown_flag_;      ///< Reference to shutdown flag.
    ResourceThreadPool& thread_pool_;       ///< Reference to resource thread pool.
    std::thread monitor_thread_;            ///< Monitor thread.
    bool running_ = false;                  ///< Indicates if the monitor is running.
};