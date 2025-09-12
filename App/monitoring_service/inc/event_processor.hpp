/**
 * @file event_processor.hpp
 * @brief Declares the EventProcessor class for processing container events and host metrics.
 */

#pragma once
#include <atomic>
#include <thread>
#include <string>
#include <unordered_map>
#include "common.hpp"
#include "event_queue.hpp"
#include "database_interface.hpp"

/**
 * @class EventProcessor
 * @brief Processes container events and periodically collects host metrics.
 *
 * Runs in a separate thread, pops events from the event queue, parses them,
 * updates the database, and collects host resource usage at regular intervals.
 */
class EventProcessor {
public:
    /**
     * @brief Constructs an EventProcessor.
     * @param queue Reference to the event queue.
     * @param shutdown_flag Reference to the application's shutdown flag.
     * @param db Reference to the database interface.
     * @param cfg Reference to the monitor configuration.
     */
    EventProcessor(EventQueue& queue, std::atomic<bool>& shutdown_flag, IDatabaseInterface& db, const MonitorConfig& cfg);

    /**
     * @brief Destructor. Ensures the worker thread is stopped.
     */
    ~EventProcessor();

    /**
     * @brief Starts the event processor thread.
     */
    void start();

    /**
     * @brief Stops the event processor thread.
     */
    void stop();

private:
    /**
     * @brief Worker thread function. Processes events and collects host metrics.
     */
    void processLoop();

    EventQueue& queue_;                    ///< Reference to the event queue.
    std::atomic<bool>& shutdown_flag_;     ///< Reference to shutdown flag.
    IDatabaseInterface& db_;               ///< Reference to database interface.
    std::thread worker_;                   ///< Worker thread for event processing.
    bool running_ = false;                 ///< Indicates if the processor is running.
    const MonitorConfig& cfg_;             ///< Reference to monitor configuration.
    std::unordered_map<std::string, std::string> name_to_id; ///< Container name to ID mapping.
};