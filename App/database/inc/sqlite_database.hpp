#pragma once
#include "database_interface.hpp"
#include <sqlite3.h>
#include <string>
#include <tuple>
#include <map>

class SQLiteDatabase : public IDatabaseInterface {
public:
    SQLiteDatabase(const std::string& db_path);
    ~SQLiteDatabase();

    void saveContainer(const std::string& name, const std::tuple<std::string, double, int, int>& data) override;
    std::tuple<std::string, double, int, int> getContainer(const std::string& name) const override;
    size_t size() const override;
    const std::map<std::string, std::tuple<std::string, double, int, int>>& getAll() const override;

private:
    sqlite3* db_;
    mutable std::map<std::string, std::tuple<std::string, double, int, int>> cache_;
    void loadCache() const;
};