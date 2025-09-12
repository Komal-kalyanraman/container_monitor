#include "initializer.hpp"
#include <csignal>
#include <cerrno>
#include <mqueue.h>
#include "logger.hpp"
#include "config_parser.hpp"

void Initializer::initLogger(int argc, char* argv[], const MonitorConfig& cfg) {
    google::InitGoogleLogging(argv[0]);
    // Optionally set FLAGS_log_dir, etc. from cfg
}

void Initializer::unlinkMessageQueue() {
    int unlink_result = mq_unlink(METRIC_MQ_NAME.data());
    if (unlink_result == 0) {
        CM_LOG_INFO << "[Main] Successfully unlinked message queue \n";
    } else {
        CM_LOG_ERROR << "[Main] mq_unlink failed: " << strerror(errno) << "\n";
    }
}

MonitorConfig Initializer::parseConfig() {
    ConfigParser parser;
    if (!parser.load(CONFIG_FILE_PATH)) {
        CM_LOG_FATAL << "Failed to load configuration file. Exiting application \n";
    }
    MonitorConfig cfg = parser.toMonitorConfig();
    parser.printConfig(cfg);
    return cfg;
}

void Initializer::setupSignalHandlers(void (*handler)(int)) {
    std::signal(SIGINT, handler);
    std::signal(SIGTERM, handler);
}