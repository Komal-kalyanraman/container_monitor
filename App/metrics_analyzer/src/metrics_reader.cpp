#include "metrics_reader.hpp"
#include <fstream>
#include <sys/sysinfo.h>
#include <string>
#include <cmath>
#include <unistd.h>

MetricsReader::MetricsReader(const ContainerResourcePaths& paths, int num_cpus)
    : paths_(paths), num_cpus_(num_cpus)
{}

uint64_t MetricsReader::readUintFromFile(const std::string& path) {
    std::ifstream file(path);
    uint64_t value = 0;
    if (file.is_open()) {
        file >> value;
    }
    return value;
}

int MetricsReader::getMemoryUsage() {
    uint64_t mem_bytes = readUintFromFile(paths_.memory_path);
    return static_cast<int>(mem_bytes / (1024 * 1024)); // MB
}

int MetricsReader::getPids() {
    return static_cast<int>(readUintFromFile(paths_.pids_path));
}

double MetricsReader::round2(double val) const {
    return std::round(val * 100.0) / 100.0;
}

double MetricsReader::getMemoryUsagePercent(const ContainerInfo& info) {
    int mem_mb = getMemoryUsage();
    double percent = (info.memory_limit > 0) ? ((double)mem_mb / info.memory_limit * 100.0) : 0.0;
    return round2(percent);
}

double MetricsReader::getPidsPercent(const ContainerInfo& info) {
    int pids = getPids();
    double percent = (info.pid_limit > 0) ? ((double)pids / info.pid_limit * 100.0) : 0.0;
    return round2(percent);
}

// Host metrics
HostInfo MetricsReader::getHostInfo() {
    HostInfo info;
    info.num_cpus = get_nprocs();
    struct sysinfo s_info;
    sysinfo(&s_info);
    info.total_memory_mb = s_info.totalram / (1024 * 1024);
    return info;
}

static uint64_t last_total = 0, last_idle = 0;

double MetricsReader::getHostCpuUsagePercentage() {
    std::ifstream file("/proc/stat");
    std::string line;
    std::getline(file, line);
    uint64_t user, nice, system, idle, iowait, irq, softirq, steal;
    sscanf(line.c_str(), "cpu  %lu %lu %lu %lu %lu %lu %lu %lu",
           &user, &nice, &system, &idle, &iowait, &irq, &softirq, &steal);
    uint64_t total = user + nice + system + idle + iowait + irq + softirq + steal;
    uint64_t total_idle = idle + iowait;

    double usage = 0.0;
    if (last_total != 0 && last_idle != 0) {
        uint64_t delta_total = total - last_total;
        uint64_t delta_idle = total_idle - last_idle;
        usage = (double)(delta_total - delta_idle) / delta_total * 100.0;
        usage = std::round(usage * 100.0) / 100.0;
    }
    last_total = total;
    last_idle = total_idle;
    return usage;
}

double MetricsReader::getHostMemoryUsagePercent() {
    std::ifstream file("/proc/meminfo");
    std::string line;
    uint64_t mem_total = 0, mem_free = 0, buffers = 0, cached = 0;
    while (std::getline(file, line)) {
        if (line.find("MemTotal:") == 0)
            sscanf(line.c_str(), "MemTotal: %lu kB", &mem_total);
        else if (line.find("MemFree:") == 0)
            sscanf(line.c_str(), "MemFree: %lu kB", &mem_free);
        else if (line.find("Buffers:") == 0)
            sscanf(line.c_str(), "Buffers: %lu kB", &buffers);
        else if (line.find("Cached:") == 0)
            sscanf(line.c_str(), "Cached: %lu kB", &cached);
    }
    uint64_t used = mem_total - mem_free - buffers - cached;
    double percent = (mem_total > 0) ? ((double)used / mem_total * 100.0) : 0.0;
    return std::round(percent * 100.0) / 100.0;
}