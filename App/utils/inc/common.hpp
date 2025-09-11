#pragma once
#include <string>
#include <cstring>
#include <string_view>

// Configuration structure
struct MonitorConfig {
    std::string runtime;
    std::string cgroup;
    std::string database;
    int resource_sampling_interval_ms;
    int container_event_refresh_interval_ms;
    std::string db_path;
    bool ui_enabled;
    int batch_size;
    double alert_warning;
    double alert_critical;
    double alert_violation;
    int thread_count;
    int thread_capacity;
    std::string file_export_folder_path;
    int ui_refresh_interval_ms;
};

// Metrics structure for storing data as percentages
struct ContainerMetrics {
    int64_t timestamp;
    double cpu_usage_percent;
    double memory_usage_percent;
    double pids_percent;
};

#pragma pack(push, 1)
// Message structure for POSIX message queue
struct ContainerMaxMetricsMsg {
    double max_cpu_usage_percent;
    double max_memory_usage_percent;
    double max_pids_percent;
    char container_id[100];
};
#pragma pack(pop)

// Structure for container resource paths
struct ContainerResourcePaths {
    std::string cpu_path;
    std::string memory_path;
    std::string pids_path;
};

// Structure for container limits information
struct ContainerInfo {
    std::string id;
    double cpu_limit;
    int memory_limit;
    int pid_limit;
};

// Structure for host system information
struct HostInfo {
    int num_cpus;
    uint64_t total_memory_mb;
};

// Unit conversion and percentage constants
constexpr double NANOSECONDS_PER_SECOND = 1e9;
constexpr double MILLISECONDS_PER_SECOND = 1000.0;
constexpr double PERCENT_FACTOR = 100.0;
constexpr double ZERO_PERCENT = 0.0;
constexpr uint64_t BYTES_PER_KILOBYTE = 1024;
constexpr uint64_t KILOBYTES_PER_MEGABYTE = 1024;

// Sleep duration for main loop in milliseconds
inline constexpr int MAIN_LOOP_SLEEP_MS = 100;

// Message queue constants
inline constexpr std::string_view METRIC_MQ_NAME = "/container_max_metric_mq";
inline constexpr size_t METRIC_MQ_MSG_SIZE = sizeof(ContainerMaxMetricsMsg);
inline constexpr long METRIC_MQ_MAX_MSG = 100; // fixed size queue

// Configuration file path
inline constexpr std::string_view CONFIG_FILE_PATH = "../../config/parameter.conf";

// Config keys as string_view
inline constexpr std::string_view KEY_RUNTIME = "runtime";
inline constexpr std::string_view KEY_CGROUP = "cgroup";
inline constexpr std::string_view KEY_DATABASE = "database";
inline constexpr std::string_view KEY_RESOURCE_SAMPLING_INTERVAL_MS = "resource_sampling_interval_ms";
inline constexpr std::string_view KEY_CONTAINER_EVENT_REFRESH_INTERVAL_MS = "container_event_refresh_interval_ms";
inline constexpr std::string_view KEY_DB_PATH = "db_path";
inline constexpr std::string_view KEY_UI_ENABLED = "ui_enabled";
inline constexpr std::string_view KEY_BATCH_SIZE = "batch_size";
inline constexpr std::string_view KEY_ALERT_WARNING = "alert_warning";
inline constexpr std::string_view KEY_ALERT_CRITICAL = "alert_critical";
inline constexpr std::string_view KEY_ALERT_VIOLATION = "alert_violation";
inline constexpr std::string_view KEY_THREAD_COUNT = "thread_count";
inline constexpr std::string_view KEY_THREAD_CAPACITY = "thread_capacity";
inline constexpr std::string_view KEY_FILE_EXPORT_FOLDER_PATH = "file_export_folder_path";
inline constexpr std::string_view KEY_UI_REFRESH_INTERVAL_MS = "ui_refresh_interval_ms";

// Default values as string_view
inline constexpr std::string_view DEFAULT_RUNTIME = "docker";
inline constexpr std::string_view DEFAULT_CGROUP = "v2";
inline constexpr std::string_view DEFAULT_DATABASE = "sqlite";
inline constexpr std::string_view DEFAULT_DB_PATH = "../../storage/metrics.db";
inline constexpr int DEFAULT_RESOURCE_SAMPLING_INTERVAL_MS = 500;
inline constexpr int DEFAULT_CONTAINER_EVENT_REFRESH_INTERVAL_MS = 1000;
inline constexpr bool DEFAULT_UI_ENABLED = true;
inline constexpr int DEFAULT_BATCH_SIZE = 50;
inline constexpr double DEFAULT_ALERT_WARNING = 80.0;
inline constexpr double DEFAULT_ALERT_CRITICAL = 95.0;
inline constexpr double DEFAULT_ALERT_VIOLATION = 100.0;
inline constexpr int DEFAULT_THREAD_COUNT = 5;
inline constexpr int DEFAULT_THREAD_CAPACITY = 10;
inline constexpr int DEFAULT_UI_REFRESH_INTERVAL_MS = 2000;

// UI Table Column Names
inline constexpr const char* COL_CONTAINER_NAME = "Container Name";
inline constexpr const char* COL_MAX_CPU = "Max CPU %";
inline constexpr const char* COL_MAX_MEM = "Max Memory %";
inline constexpr const char* COL_MAX_PIDS = "Max PIDs %";