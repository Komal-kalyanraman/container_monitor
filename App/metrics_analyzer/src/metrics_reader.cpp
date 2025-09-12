/**
 * @file metrics_reader.cpp
 * @brief Implements the MetricsReader class for reading container and host metrics.
 */

#include "metrics_reader.hpp"
#include <fstream>
#include <sys/sysinfo.h>
#include <string>
#include <cmath>
#include <unistd.h>
#include "common.hpp"

/**
 * @brief Constructs a MetricsReader for a specific container.
 * @param paths Resource file paths for the container.
 * @param num_cpus Number of CPUs on the host.
 */
MetricsReader::MetricsReader(const ContainerResourcePaths& paths, int num_cpus)
    : paths_(paths), num_cpus_(num_cpus)
{}

/**
 * @brief Reads an unsigned integer value from a file.
 * @param path File path.
 * @return Value read from file.
 */
uint64_t MetricsReader::readUintFromFile(const std::string& path) {
    std::ifstream file(path);
    uint64_t value = 0;
    if (file.is_open()) {
        file >> value;
    }
    return value;
}

/**
 * @brief Gets the container's memory usage in megabytes.
 * @return Memory usage in MB.
 */
int MetricsReader::getMemoryUsage() {
    uint64_t mem_bytes = readUintFromFile(paths_.memory_path);
    return static_cast<int>(mem_bytes / (BYTES_PER_KILOBYTE * KILOBYTES_PER_MEGABYTE)); // MB
}

/**
 * @brief Gets the container's current pids count.
 * @return Number of pids.
 */
int MetricsReader::getPids() {
    return static_cast<int>(readUintFromFile(paths_.pids_path));
}

/**
 * @brief Rounds a value to two decimal places.
 * @param val Value to round.
 * @return Rounded value.
 */
double MetricsReader::round2(double val) const {
    return std::round(val * PERCENT_FACTOR) / PERCENT_FACTOR;
}

/**
 * @brief Gets the container's memory usage as a percentage of its limit.
 * @param info ContainerInfo struct.
 * @return Memory usage percent.
 */
double MetricsReader::getMemoryUsagePercent(const ContainerInfo& info) {
    int mem_mb = getMemoryUsage();
    double percent = (info.memory_limit > 0) ? ((double)mem_mb / info.memory_limit * PERCENT_FACTOR) : ZERO_PERCENT;
    return round2(percent);
}

/**
 * @brief Gets the container's pids usage as a percentage of its limit.
 * @param info ContainerInfo struct.
 * @return Pids usage percent.
 */
double MetricsReader::getPidsPercent(const ContainerInfo& info) {
    int pids = getPids();
    double percent = (info.pid_limit > 0) ? ((double)pids / info.pid_limit * PERCENT_FACTOR) : ZERO_PERCENT;
    return round2(percent);
}

/**
 * @brief Gets host system information (CPU count, total memory).
 * @return HostInfo struct.
 */
HostInfo MetricsReader::getHostInfo() {
    HostInfo info;
    info.num_cpus = get_nprocs();
    struct sysinfo s_info;
    sysinfo(&s_info);
    info.total_memory_mb = s_info.totalram / (BYTES_PER_KILOBYTE * KILOBYTES_PER_MEGABYTE);
    return info;
}

/**
 * @brief Gets host CPU usage percentage.
 * @return CPU usage percent.
 */
double MetricsReader::getHostCpuUsagePercentage() {
    std::ifstream file(PROC_STAT_PATH);
    std::string line;
    std::getline(file, line);
    uint64_t user, nice, system, idle, iowait, irq, softirq, steal;
    sscanf(line.c_str(), CPU_STAT_FORMAT,
           &user, &nice, &system, &idle, &iowait, &irq, &softirq, &steal);
    uint64_t total = user + nice + system + idle + iowait + irq + softirq + steal;
    uint64_t total_idle = idle + iowait;

    double usage = ZERO_PERCENT;
    if (last_total != 0 && last_idle != 0) {
        uint64_t delta_total = total - last_total;
        uint64_t delta_idle = total_idle - last_idle;
        usage = (double)(delta_total - delta_idle) / delta_total * PERCENT_FACTOR;
        usage = std::round(usage * PERCENT_FACTOR) / PERCENT_FACTOR;
    }
    last_total = total;
    last_idle = total_idle;
    return usage;
}

/**
 * @brief Gets host memory usage percentage.
 * @return Memory usage percent.
 */
double MetricsReader::getHostMemoryUsagePercent() {
    std::ifstream file(PROC_MEMINFO_PATH);
    std::string line;
    uint64_t mem_total = 0, mem_free = 0, buffers = 0, cached = 0;
    while (std::getline(file, line)) {
        if (line.find(MEMINFO_TOTAL) == 0)
            sscanf(line.c_str(), (std::string(MEMINFO_TOTAL) + " " + MEMINFO_FORMAT).c_str(), &mem_total);
        else if (line.find(MEMINFO_FREE) == 0)
            sscanf(line.c_str(), (std::string(MEMINFO_FREE) + " " + MEMINFO_FORMAT).c_str(), &mem_free);
        else if (line.find(MEMINFO_BUFFERS) == 0)
            sscanf(line.c_str(), (std::string(MEMINFO_BUFFERS) + " " + MEMINFO_FORMAT).c_str(), &buffers);
        else if (line.find(MEMINFO_CACHED) == 0)
            sscanf(line.c_str(), (std::string(MEMINFO_CACHED) + " " + MEMINFO_FORMAT).c_str(), &cached);
    }
    uint64_t used = mem_total - mem_free - buffers - cached;
    double percent = (mem_total > 0) ? ((double)used / mem_total * PERCENT_FACTOR) : ZERO_PERCENT;
    return std::round(percent * PERCENT_FACTOR) / PERCENT_FACTOR;
}