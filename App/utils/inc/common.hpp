#pragma once
#include <string>
#include <string_view>

struct MonitorConfig {
    std::string runtime;
    std::string cgroup;
    std::string database;
    int sampling_interval_ms;
    std::string db_path;
    bool ui_enabled;
    int batch_size;
    double alert_warning;
    double alert_critical;
    double alert_violation;
};

// Configuration file path
inline constexpr std::string_view CONFIG_FILE_PATH = "../../config/parameter.conf";

// Config keys as string_view
inline constexpr std::string_view KEY_RUNTIME = "runtime";
inline constexpr std::string_view KEY_CGROUP = "cgroup";
inline constexpr std::string_view KEY_DATABASE = "database";
inline constexpr std::string_view KEY_SAMPLING_INTERVAL_MS = "sampling_interval_ms";
inline constexpr std::string_view KEY_DB_PATH = "db_path";
inline constexpr std::string_view KEY_UI_ENABLED = "ui_enabled";
inline constexpr std::string_view KEY_BATCH_SIZE = "batch_size";
inline constexpr std::string_view KEY_ALERT_WARNING = "alert_warning";
inline constexpr std::string_view KEY_ALERT_CRITICAL = "alert_critical";
inline constexpr std::string_view KEY_ALERT_VIOLATION = "alert_violation";

// Default values as string_view
inline constexpr std::string_view DEFAULT_RUNTIME = "docker";
inline constexpr std::string_view DEFAULT_CGROUP = "v2";
inline constexpr std::string_view DEFAULT_DATABASE = "sqlite";
inline constexpr std::string_view DEFAULT_DB_PATH = "../../storage/metrics.db";
inline constexpr int DEFAULT_SAMPLING_INTERVAL_MS = 500;
inline constexpr bool DEFAULT_UI_ENABLED = true;
inline constexpr int DEFAULT_BATCH_SIZE = 50;
inline constexpr double DEFAULT_ALERT_WARNING = 80.0;
inline constexpr double DEFAULT_ALERT_CRITICAL = 95.0;
inline constexpr double DEFAULT_ALERT_VIOLATION = 100.0;