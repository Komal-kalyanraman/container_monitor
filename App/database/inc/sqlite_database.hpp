#pragma once
#include "database_interface.hpp"
#include <sqlite3.h>
#include <string>
#include <tuple>
#include <map>
#include <mutex>
#include <vector>
#include "common.hpp"

class SQLiteDatabase : public IDatabaseInterface {
public:
    SQLiteDatabase(const std::string& db_path);
    ~SQLiteDatabase();

    void saveContainer(const std::string& name, const ContainerInfo& info) override;
    void removeContainer(const std::string& name) override;
    void clearAll() override;
    ContainerInfo getContainer(const std::string& name) const override;
    size_t size() const override;
    const std::map<std::string, ContainerInfo>& getAll() const override;
    void setupSchema() override;
    void insertBatch(const std::string& container_name, const std::vector<ResourceSample>& samples) override;
    void exportToCSV(const std::string& filename) override;
    void saveHostUsage(int64_t timestamp_ms, double cpu_usage, uint64_t mem_usage_mb) override;

private:
    sqlite3* db_;
    mutable std::mutex db_mutex; 
    mutable std::map<std::string, ContainerInfo> cache_;
    void loadCache() const;
};