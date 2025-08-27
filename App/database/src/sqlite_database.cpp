#include "sqlite_database.hpp"
#include <iostream>

SQLiteDatabase::SQLiteDatabase(const std::string& db_path) : db_(nullptr) {
    if (sqlite3_open(db_path.c_str(), &db_) != SQLITE_OK) {
        std::cerr << "Failed to open SQLite database: " << db_path << std::endl;
        db_ = nullptr;
    } else {
        const char* create_table_sql =
            "CREATE TABLE IF NOT EXISTS containers ("
            "name TEXT PRIMARY KEY,"
            "id TEXT,"
            "cpus REAL,"
            "memory INTEGER,"
            "pids_limit INTEGER"
            ");";
        char* err_msg = nullptr;
        if (sqlite3_exec(db_, create_table_sql, nullptr, nullptr, &err_msg) != SQLITE_OK) {
            std::cerr << "Failed to create table: " << err_msg << std::endl;
            sqlite3_free(err_msg);
        }
    }
}

SQLiteDatabase::~SQLiteDatabase() {
    if (db_) sqlite3_close(db_);
}

void SQLiteDatabase::saveContainer(const std::string& name, const std::tuple<std::string, double, int, int>& data) {
    if (!db_) return;
    const char* sql = "INSERT OR REPLACE INTO containers (name, id, cpus, memory, pids_limit) VALUES (?, ?, ?, ?, ?);";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, std::get<0>(data).c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_double(stmt, 3, std::get<1>(data));
        sqlite3_bind_int(stmt, 4, std::get<2>(data));
        sqlite3_bind_int(stmt, 5, std::get<3>(data));
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    }
    cache_[name] = data;
}

std::tuple<std::string, double, int, int> SQLiteDatabase::getContainer(const std::string& name) const {
    if (!db_) return {};
    loadCache();
    auto it = cache_.find(name);
    if (it != cache_.end()) return it->second;
    return {};
}

size_t SQLiteDatabase::size() const {
    loadCache();
    return cache_.size();
}

const std::map<std::string, std::tuple<std::string, double, int, int>>& SQLiteDatabase::getAll() const {
    loadCache();
    return cache_;
}

void SQLiteDatabase::loadCache() const {
    if (!db_) return;
    cache_.clear();
    const char* sql = "SELECT name, id, cpus, memory, pids_limit FROM containers;";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            std::string name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
            std::string id = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
            double cpus = sqlite3_column_double(stmt, 2);
            int memory = sqlite3_column_int(stmt, 3);
            int pids_limit = sqlite3_column_int(stmt, 4);
            cache_[name] = std::make_tuple(id, cpus, memory, pids_limit);
        }
        sqlite3_finalize(stmt);
    }
}