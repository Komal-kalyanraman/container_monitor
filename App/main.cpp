#include <iostream>
#include "logger.hpp"
#include "common.hpp"
#include "config_parser.hpp"

int main() {
    ConfigParser parser;
    if (!parser.load(CONFIG_FILE_PATH)) {
        CM_LOG_ERROR << "Failed to load configuration file.\n";
        return 1;
    }
    MonitorConfig cfg = parser.toMonitorConfig();
    CM_LOG_INFO << "Container Monitor started.\n";
    CM_LOG_INFO << "Runtime: " << cfg.runtime << "\n";
    CM_LOG_INFO << "Sampling interval: " << cfg.sampling_interval_ms << " ms\n";
    CM_LOG_INFO << "DB Path: " << cfg.db_path << "\n";
    CM_LOG_INFO << "UI Enabled: " << (cfg.ui_enabled ? "true" : "false") << "\n";
    CM_LOG_INFO << "Batch Size: " << cfg.batch_size << "\n";
    CM_LOG_INFO << "Alert thresholds: warning=" << cfg.alert_warning
                << ", critical=" << cfg.alert_critical
                << ", violation=" << cfg.alert_violation << "\n";
    // TODO: Pass cfg to modules and start monitoring loop
    return 0;
}