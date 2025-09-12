/**
 * @file metrics_reader.hpp
 * @brief Declares the MetricsReader class for reading container and host metrics.
 */

#pragma once
#include <string>
#include "common.hpp"

/**
 * @class MetricsReader
 * @brief Reads resource usage metrics for containers and the host system.
 *
 * Provides methods to read memory, pids, and CPU usage for containers,
 * as well as host CPU and memory usage. Supports conversion to percentages.
 */
class MetricsReader {
public:
    /**
     * @brief Constructs a MetricsReader for a specific container.
     * @param paths Resource file paths for the container.
     * @param num_cpus Number of CPUs on the host.
     */
    MetricsReader(const ContainerResourcePaths& paths, int num_cpus);

    // Container metrics

    /**
     * @brief Gets the container's memory usage in megabytes.
     * @return Memory usage in MB.
     */
    int getMemoryUsage();

    /**
     * @brief Gets the container's current pids count.
     * @return Number of pids.
     */
    int getPids();

    /**
     * @brief Gets the container's memory usage as a percentage of its limit.
     * @param info ContainerInfo struct.
     * @return Memory usage percent.
     */
    double getMemoryUsagePercent(const ContainerInfo& info);

    /**
     * @brief Gets the container's pids usage as a percentage of its limit.
     * @param info ContainerInfo struct.
     * @return Pids usage percent.
     */
    double getPidsPercent(const ContainerInfo& info);

    // Host metrics (static methods)

    /**
     * @brief Gets host system information (CPU count, total memory).
     * @return HostInfo struct.
     */
    HostInfo getHostInfo();

    /**
     * @brief Gets host CPU usage percentage.
     * @return CPU usage percent.
     */
    double getHostCpuUsagePercentage();

    /**
     * @brief Gets host memory usage percentage.
     * @return Memory usage percent.
     */
    double getHostMemoryUsagePercent();

    /**
     * @brief Reads an unsigned integer value from a file.
     * @param path File path.
     * @return Value read from file.
     */
    uint64_t readUintFromFile(const std::string& path);

private:
    ContainerResourcePaths paths_;      ///< Resource file paths for the container.
    double round2(double val) const;    ///< Rounds a value to two decimal places.
    int num_cpus_;                      ///< Number of CPUs on the host.
    uint64_t last_total = 0;            ///< Last total CPU ticks (for host CPU usage).
    uint64_t last_idle = 0;             ///< Last idle CPU ticks (for host CPU usage).
};