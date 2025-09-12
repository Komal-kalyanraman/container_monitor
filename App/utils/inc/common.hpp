/**
 * @file common.hpp
 * @brief Common data structures, constants, and configuration for container monitoring.
 *
 * Defines configuration structures, metric types, resource paths, constants for unit conversion,
 * message queue parameters, SQL schema strings, CSV export filenames, and parsing tokens.
 */

#pragma once
#include <string>
#include <cstring>
#include <string_view>

/**
 * @struct MonitorConfig
 * @brief Holds configuration parameters for the container monitor application.
 */
struct MonitorConfig {
    std::string runtime;                    ///< Container runtime (e.g., docker, podman).
    std::string cgroup;                     ///< Cgroup version (e.g., v1, v2).
    std::string database;                   ///< Database type (e.g., sqlite).
    int resource_sampling_interval_ms;      ///< Resource sampling interval in milliseconds.
    int container_event_refresh_interval_ms;///< Container event refresh interval in milliseconds.
    std::string db_path;                    ///< Path to the database file.
    bool ui_enabled;                        ///< Whether the UI is enabled.
    int batch_size;                         ///< Batch size for metric inserts.
    double alert_warning;                   ///< Warning threshold for alerts.
    double alert_critical;                  ///< Critical threshold for alerts.
    int thread_count;                       ///< Number of resource threads.
    int thread_capacity;                    ///< Maximum containers per thread.
    std::string file_export_folder_path;    ///< Path for CSV exports.
    int ui_refresh_interval_ms;             ///< UI refresh interval in milliseconds.
};

/**
 * @struct ContainerMetrics
 * @brief Stores resource usage metrics for a container as percentages.
 */
struct ContainerMetrics {
    int64_t timestamp;              ///< Timestamp in milliseconds.
    double cpu_usage_percent;       ///< CPU usage percent.
    double memory_usage_percent;    ///< Memory usage percent.
    double pids_percent;            ///< PIDs usage percent.
};

/**
 * @brief Buffer size for container ID in messages.
 */
inline constexpr size_t CONTAINER_ID_BUF_SIZE = 100;

#pragma pack(push, 1)
/**
 * @struct ContainerMaxMetricsMsg
 * @brief Message structure for POSIX message queue, containing max metrics for a container.
 */
struct ContainerMaxMetricsMsg {
    double max_cpu_usage_percent;           ///< Maximum CPU usage percent.
    double max_memory_usage_percent;        ///< Maximum memory usage percent.
    double max_pids_percent;                ///< Maximum PIDs usage percent.
    char container_id[CONTAINER_ID_BUF_SIZE]; ///< Container ID.
};
#pragma pack(pop)

/**
 * @struct ContainerResourcePaths
 * @brief Holds file paths for container resource usage.
 */
struct ContainerResourcePaths {
    std::string cpu_path;       ///< Path to CPU usage file.
    std::string memory_path;    ///< Path to memory usage file.
    std::string pids_path;      ///< Path to PIDs usage file.
};

/**
 * @struct ContainerInfo
 * @brief Holds resource limits for a container at the time of creation.
 */
struct ContainerInfo {
    std::string id;         ///< Container ID.
    double cpu_limit;       ///< CPU limit (cores).
    int memory_limit;       ///< Memory limit (MB).
    int pid_limit;          ///< PIDs limit.
};

/**
 * @struct HostInfo
 * @brief Holds host system information.
 */
struct HostInfo {
    int num_cpus;               ///< Number of CPUs.
    uint64_t total_memory_mb;   ///< Total memory in MB.
};

// Unit conversion and percentage constants
constexpr double NANOSECONDS_PER_SECOND = 1e9;           ///< Nanoseconds per second.
constexpr double MILLISECONDS_PER_SECOND = 1000.0;       ///< Milliseconds per second.
constexpr double PERCENT_FACTOR = 100.0;                 ///< Factor for percentage calculation.
constexpr double ZERO_PERCENT = 0.0;                     ///< Zero percent value.
constexpr uint64_t BYTES_PER_KILOBYTE = 1024;            ///< Bytes per kilobyte.
constexpr uint64_t KILOBYTES_PER_MEGABYTE = 1024;        ///< Kilobytes per megabyte.

// Sleep duration in milliseconds
inline constexpr int MAIN_LOOP_SLEEP_MS = 100;           ///< Main loop sleep duration.
inline constexpr int SLEEP_MS_SHORT   = 1;               ///< Short sleep duration.
inline constexpr int SLEEP_MS_MEDIUM  = 500;             ///< Medium sleep duration.
inline constexpr int SLEEP_MS_LONG    = 1000;            ///< Long sleep duration.

// Message queue constants
inline constexpr std::string_view METRIC_MQ_NAME = "/container_max_metric_mq"; ///< POSIX message queue name.
inline constexpr size_t METRIC_MQ_MSG_SIZE = sizeof(ContainerMaxMetricsMsg);    ///< Message size.
inline constexpr long METRIC_MQ_MAX_MSG = 100;                                 ///< Max messages in queue.

// Configuration file path
inline constexpr std::string_view CONFIG_FILE_PATH = "../../config/parameter.conf"; ///< Path to config file.

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
inline constexpr std::string_view KEY_THREAD_COUNT = "thread_count";
inline constexpr std::string_view KEY_THREAD_CAPACITY = "thread_capacity";
inline constexpr std::string_view KEY_FILE_EXPORT_FOLDER_PATH = "file_export_folder_path";
inline constexpr std::string_view KEY_UI_REFRESH_INTERVAL_MS = "ui_refresh_interval_ms";

// Default values as string_view
inline constexpr std::string_view DEFAULT_RUNTIME = "docker";
inline constexpr std::string_view DEFAULT_CGROUP = "v2";
inline constexpr std::string_view DEFAULT_DATABASE = "sqlite";
inline constexpr std::string_view DEFAULT_DB_PATH = "../../storage/metrics.db";
inline constexpr std::string_view DEFAULT_FILE_EXPORT_FOLDER_PATH = "../../storage";
inline constexpr int DEFAULT_RESOURCE_SAMPLING_INTERVAL_MS = 500;
inline constexpr int DEFAULT_CONTAINER_EVENT_REFRESH_INTERVAL_MS = 1000;
inline constexpr bool DEFAULT_UI_ENABLED = true;
inline constexpr int DEFAULT_BATCH_SIZE = 50;
inline constexpr double DEFAULT_ALERT_WARNING = 80.0;
inline constexpr double DEFAULT_ALERT_CRITICAL = 100.0;
inline constexpr int DEFAULT_THREAD_COUNT = 5;
inline constexpr int DEFAULT_THREAD_CAPACITY = 10;
inline constexpr int DEFAULT_UI_REFRESH_INTERVAL_MS = 2000;

// UI Table Column Names
inline constexpr const char* COL_CONTAINER_NAME = "Container Name"; ///< UI column: container name.
inline constexpr const char* COL_MAX_CPU = "Max CPU %";             ///< UI column: max CPU.
inline constexpr const char* COL_MAX_MEM = "Max Memory %";          ///< UI column: max memory.
inline constexpr const char* COL_MAX_PIDS = "Max PIDs %";           ///< UI column: max PIDs.

// System resource file paths
inline constexpr const char* PROC_STAT_PATH    = "/proc/stat";      ///< Path to /proc/stat.
inline constexpr const char* PROC_MEMINFO_PATH = "/proc/meminfo";   ///< Path to /proc/meminfo.

// /proc/stat parsing format
inline constexpr const char* CPU_STAT_FORMAT = "cpu  %lu %lu %lu %lu %lu %lu %lu %lu"; ///< Format for /proc/stat.

// /proc/meminfo parsing tokens
inline constexpr const char* MEMINFO_TOTAL   = "MemTotal:";         ///< Token for total memory.
inline constexpr const char* MEMINFO_FREE    = "MemFree:";          ///< Token for free memory.
inline constexpr const char* MEMINFO_BUFFERS = "Buffers:";          ///< Token for buffers.
inline constexpr const char* MEMINFO_CACHED  = "Cached:";           ///< Token for cached memory.

// /proc/meminfo parsing format
inline constexpr const char* MEMINFO_FORMAT  = "%lu kB";            ///< Format for /proc/meminfo values.

// Cgroup path buffer size
inline constexpr size_t CGROUP_PATH_BUF_SIZE = 512;                 ///< Buffer size for cgroup paths.

// Docker Cgroup v1 path formats
inline constexpr const char* DOCKER_CGROUP_V1_CPU_PATH_FMT    = "/sys/fs/cgroup/cpu/docker/%s/cpuacct.usage"; ///< Format for CPU path.
inline constexpr const char* DOCKER_CGROUP_V1_MEMORY_PATH_FMT = "/sys/fs/cgroup/memory/docker/%s/memory.usage_in_bytes"; ///< Format for memory path.
inline constexpr const char* DOCKER_CGROUP_V1_PIDS_PATH_FMT   = "/sys/fs/cgroup/pids/docker/%s/pids.current"; ///< Format for PIDs path.

// SQLite table schema and SQL statements
inline constexpr const char* SQL_CREATE_CONTAINERS_TABLE =
    "CREATE TABLE IF NOT EXISTS containers ("
    "name TEXT PRIMARY KEY,"
    "id TEXT,"
    "cpus REAL,"
    "memory REAL,"
    "pids_limit INTEGER"
    ");"; ///< SQL for creating containers table.

inline constexpr const char* SQL_CREATE_CONTAINER_METRICS_TABLE =
    "CREATE TABLE IF NOT EXISTS container_metrics ("
    "container_name TEXT,"
    "timestamp INTEGER,"
    "cpu_usage REAL,"
    "memory_usage REAL,"
    "pids INTEGER"
    ");"; ///< SQL for creating container_metrics table.

inline constexpr const char* SQL_CREATE_HOST_USAGE_TABLE =
    "CREATE TABLE IF NOT EXISTS host_usage ("
    "timestamp INTEGER,"
    "cpu_usage_percent REAL,"
    "memory_usage_percent REAL"
    ");"; ///< SQL for creating host_usage table.

inline constexpr const char* SQL_INSERT_OR_REPLACE_CONTAINER =
    "INSERT OR REPLACE INTO containers (name, id, cpus, memory, pids_limit) VALUES (?, ?, ?, ?, ?);"; ///< SQL for upserting container.

inline constexpr const char* SQL_SELECT_CONTAINER =
    "SELECT name, id, cpus, memory, pids_limit FROM containers;"; ///< SQL for selecting containers.

inline constexpr const char* SQL_DELETE_CONTAINER_BY_NAME =
    "DELETE FROM containers WHERE name = ?;"; ///< SQL for deleting container by name.

inline constexpr const char* SQL_DELETE_ALL_CONTAINERS =
    "DELETE FROM containers;"; ///< SQL for deleting all containers.

inline constexpr const char* SQL_DELETE_CONTAINER_METRICS =
    "DELETE FROM container_metrics;"; ///< SQL for deleting all container metrics.

inline constexpr const char* SQL_DELETE_HOST_USAGE =
    "DELETE FROM host_usage;"; ///< SQL for deleting all host usage.

inline constexpr const char* SQL_INSERT_CONTAINER_METRICS =
    "INSERT INTO container_metrics (container_name, timestamp, cpu_usage, memory_usage, pids) VALUES (?, ?, ?, ?, ?);"; ///< SQL for inserting container metrics.

inline constexpr const char* SQL_SELECT_CONTAINER_METRICS =
    "SELECT container_name, timestamp, cpu_usage, memory_usage, pids FROM container_metrics;"; ///< SQL for selecting container metrics.

inline constexpr const char* SQL_SELECT_HOST_USAGE =
    "SELECT timestamp, cpu_usage_percent, memory_usage_percent FROM host_usage;"; ///< SQL for selecting host usage.

inline constexpr const char* SQL_INSERT_HOST_USAGE =
    "INSERT INTO host_usage (timestamp, cpu_usage_percent, memory_usage_percent) VALUES (?, ?, ?);"; ///< SQL for inserting host usage.

// CSV export filenames
inline constexpr const char* CSV_CONTAINER_METRICS_FILENAME = "/container_metrics.csv"; ///< Filename for container metrics CSV.
inline constexpr const char* CSV_HOST_USAGE_FILENAME        = "/host_usage.csv";        ///< Filename for host usage CSV.

// CSV header strings
inline constexpr const char* CSV_CONTAINER_METRICS_HEADER = "container_name,timestamp,cpu_usage,memory_usage,pids\n"; ///< Header for container metrics CSV.
inline constexpr const char* CSV_HOST_USAGE_HEADER        = "timestamp,cpu_usage_percent,memory_usage_percent\n";     ///< Header for host usage CSV.