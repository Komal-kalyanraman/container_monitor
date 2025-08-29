#pragma once
#include <tuple>
#include <vector>
#include <map>
#include <string>
#include "common.hpp"

class IDatabaseInterface {
public:
    virtual ~IDatabaseInterface() = default;
    virtual void saveContainer(const std::string& name, const std::tuple<std::string, double, int, int>& data) = 0;
    virtual void insertBatch(const std::string& container_name, const std::vector<ResourceSample>& samples) = 0;
    virtual void removeContainer(const std::string& name) = 0;
    virtual void clearAll() = 0;
    virtual std::tuple<std::string, double, int, int> getContainer(const std::string& name) const = 0;
    virtual size_t size() const = 0;
    virtual const std::map<std::string, std::tuple<std::string, double, int, int>>& getAll() const = 0;
    virtual void initialize() = 0;
    virtual void exportToCSV(const std::string& filename) = 0;
};