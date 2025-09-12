/**
 * @file initializer.cpp
 * @brief Implements the Initializer class for application startup routines.
 */

#include "initializer.hpp"
#include <csignal>
#include <cerrno>
#include <mqueue.h>
#include "logger.hpp"
#include "config_parser.hpp"

/**
 * @brief Initializes the glog logger.
 * @param argc Command-line argument count.
 * @param argv Command-line argument array.
 * @param cfg Monitor configuration.
 */
void Initializer::initLogger(int argc, char* argv[], const MonitorConfig& cfg) {
    google::InitGoogleLogging(argv[0]);
    // Optionally set FLAGS_log_dir, etc. from cfg
}

/**
 * @brief Unlinks the POSIX message queue used for metrics.
 */
void Initializer::unlinkMessageQueue() {
    int unlink_result = mq_unlink(METRIC_MQ_NAME.data());
    if (unlink_result == 0) {
        CM_LOG_INFO << "[Main] Successfully unlinked message queue \n";
    } else {
        CM_LOG_ERROR << "[Main] mq_unlink failed: " << strerror(errno) << "\n";
    }
}

/**
 * @brief Parses the configuration file and returns a MonitorConfig object.
 * @return Parsed MonitorConfig.
 */
MonitorConfig Initializer::parseConfig() {
    ConfigParser parser;
    if (!parser.load(CONFIG_FILE_PATH)) {
        CM_LOG_FATAL << "Failed to load configuration file. Exiting application \n";
    }
    MonitorConfig cfg = parser.toMonitorConfig();
    parser.printConfig(cfg);
    return cfg;
}

/**
 * @brief Sets up signal handlers for graceful shutdown.
 * @param handler Signal handler function pointer.
 */
void Initializer::setupSignalHandlers(void (*handler)(int)) {
    std::signal(SIGINT, handler);
    std::signal(SIGTERM, handler);
}