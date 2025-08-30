#pragma once
#include <string>
#include "common.hpp"

class MetricsReader {
public:
    MetricsReader(const ContainerResourcePaths& paths, int num_cpus);

    // Container metrics
    double getCpuUsage();      // % per core
    int getMemoryUsage();      // MB
    int getPids();             // count

    // Host metrics (static methods)
    static HostInfo getHostInfo();
    static double getHostCpuUsage();         // % (since last call)
    static uint64_t getHostMemoryUsageMB();  // MB

private:
    ContainerResourcePaths paths_;
    int num_cpus_;

    uint64_t readUintFromFile(const std::string& path);
};