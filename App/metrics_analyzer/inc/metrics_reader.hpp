#pragma once
#include <string>
#include "common.hpp"

class MetricsReader {
public:
    MetricsReader(const ContainerResourcePaths& paths, int num_cpus);

    // Container metrics
    int getMemoryUsage();      // MB
    int getPids();             // count
    double getMemoryUsagePercent(const ContainerInfo& info);
    double getPidsPercent(const ContainerInfo& info);

    // Host metrics (static methods)
    HostInfo getHostInfo();
    double getHostCpuUsagePercentage();    // %
    double getHostMemoryUsagePercent();    // % 

    uint64_t readUintFromFile(const std::string& path);


private:
    ContainerResourcePaths paths_;
    double round2(double val) const;
    int num_cpus_;
};