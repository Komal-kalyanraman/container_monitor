#pragma once
#include <tuple>
#include <vector>
#include <map>
#include <string>
#include "common.hpp"

class IDatabaseInterface {
public:
    virtual ~IDatabaseInterface() = default;
    virtual void saveContainer(const std::string& name, const ContainerInfo& info) = 0;
    virtual ContainerInfo getContainer(const std::string& name) const = 0;
    virtual void insertBatch(const std::string& container_name, const std::vector<ContainerMetrics>& metrics_vec) = 0;
    virtual void removeContainer(const std::string& name) = 0;
    virtual void clearAll() = 0;
    virtual size_t size() const = 0;
    virtual const std::map<std::string, ContainerInfo>& getAll() const = 0;
    virtual void setupSchema() = 0;
    virtual void exportAllTablesToCSV(const std::string& export_dir) = 0;
    virtual void saveHostUsage(int64_t timestamp_ms, double cpu_usage, uint64_t mem_usage_mb) = 0;
};