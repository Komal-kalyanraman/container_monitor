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

    void saveContainer(const std::string& name, const std::tuple<std::string, double, int, int>& data) override;
    void removeContainer(const std::string& name) override;
    void clearAll();
    std::tuple<std::string, double, int, int> getContainer(const std::string& name) const override;
    size_t size() const override;
    const std::map<std::string, std::tuple<std::string, double, int, int>>& getAll() const override;
    void initialize();
    void insertBatch(const std::string& container_name, const std::vector<ResourceSample>& samples);

private:
    sqlite3* db_;
    mutable std::mutex db_mutex; 
    mutable std::map<std::string, std::tuple<std::string, double, int, int>> cache_;
    void loadCache() const;
};