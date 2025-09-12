/**
 * @file initializer.hpp
 * @brief Declares the Initializer class for application startup routines.
 */

#pragma once
#include "common.hpp"

/**
 * @class Initializer
 * @brief Provides static methods for initializing logging, configuration, message queues, and signal handlers.
 */
class Initializer {
public:
    /**
     * @brief Initializes the glog logger.
     * @param argc Command-line argument count.
     * @param argv Command-line argument array.
     * @param cfg Monitor configuration.
     */
    static void initLogger(int argc, char* argv[], const MonitorConfig& cfg);

    /**
     * @brief Unlinks the POSIX message queue used for metrics.
     */
    static void unlinkMessageQueue();

    /**
     * @brief Parses the configuration file and returns a MonitorConfig object.
     * @return Parsed MonitorConfig.
     */
    static MonitorConfig parseConfig();

    /**
     * @brief Sets up signal handlers for graceful shutdown.
     * @param handler Signal handler function pointer.
     */
    static void setupSignalHandlers(void (*handler)(int));
};