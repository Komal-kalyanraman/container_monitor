#include "sqlite_database.hpp"
#include <iostream>
#include <fstream>
#include <filesystem>
#include "logger.hpp"

SQLiteDatabase::SQLiteDatabase(const std::string& db_path) : db_(nullptr) {
    if (sqlite3_open(db_path.c_str(), &db_) != SQLITE_OK) {
        CM_LOG_ERROR << "Failed to open SQLite database: " << db_path << "\n";
        db_ = nullptr;
    }
}

SQLiteDatabase::~SQLiteDatabase() {
    if (db_) sqlite3_close(db_);
}

void SQLiteDatabase::saveContainer(const std::string& name, const ContainerInfo& info) {
    if (!db_) return;
    const char* sql = "INSERT OR REPLACE INTO containers (name, id, cpus, memory, pids_limit) VALUES (?, ?, ?, ?, ?);";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, info.id.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_double(stmt, 3, info.cpus);
        sqlite3_bind_int(stmt, 4, info.memory);
        sqlite3_bind_int(stmt, 5, info.pids_limit);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    }
    cache_[name] = info;
}

ContainerInfo SQLiteDatabase::getContainer(const std::string& name) const {
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

const std::map<std::string, ContainerInfo>& SQLiteDatabase::getAll() const {
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
            ContainerInfo info;
            info.id = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
            info.cpus = sqlite3_column_double(stmt, 2);
            info.memory = sqlite3_column_int(stmt, 3);
            info.pids_limit = sqlite3_column_int(stmt, 4);
            cache_[name] = info;
        }
        sqlite3_finalize(stmt);
    }
}

void SQLiteDatabase::removeContainer(const std::string& name) {
    if (!db_) return;
    const char* sql = "DELETE FROM containers WHERE name = ?;";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    }
    cache_.erase(name);
}

void SQLiteDatabase::clearAll() {
    if (!db_) return;
    const char* sql1 = "DELETE FROM containers;";
    const char* sql2 = "DELETE FROM resource_samples;";
    char* err_msg = nullptr;
    if (sqlite3_exec(db_, sql1, nullptr, nullptr, &err_msg) != SQLITE_OK) {
        CM_LOG_ERROR << "Failed to clear containers table: " << err_msg << "\n";
        sqlite3_free(err_msg);
    }
    if (sqlite3_exec(db_, sql2, nullptr, nullptr, &err_msg) != SQLITE_OK) {
        CM_LOG_ERROR << "Failed to clear resource_samples table: " << err_msg << "\n";
        sqlite3_free(err_msg);
    }
    cache_.clear();
}

void SQLiteDatabase::setupSchema() {
    // Create containers table
    const char* create_containers_sql =
        "CREATE TABLE IF NOT EXISTS containers ("
        "name TEXT PRIMARY KEY,"
        "id TEXT,"
        "cpus REAL,"
        "memory INTEGER,"
        "pids_limit INTEGER"
        ");";
    char* errMsg = nullptr;
    int rc = sqlite3_exec(db_, create_containers_sql, nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        CM_LOG_ERROR << "Failed to create containers table: " << errMsg << "\n";
        sqlite3_free(errMsg);
    }

    // Create resource_samples table
    const char* create_resource_samples_sql =
        "CREATE TABLE IF NOT EXISTS resource_samples ("
        "container_name TEXT,"
        "timestamp INTEGER,"
        "cpu_usage REAL,"
        "memory_usage REAL,"
        "pids INTEGER"
        ");";
    rc = sqlite3_exec(db_, create_resource_samples_sql, nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        CM_LOG_ERROR << "Failed to create resource_samples table: " << errMsg << "\n";
        sqlite3_free(errMsg);
    }

    // Create host_usage table
    const char* create_host_usage_sql =
        "CREATE TABLE IF NOT EXISTS host_usage ("
        "timestamp INTEGER,"
        "cpu_usage REAL,"
        "memory_usage_mb INTEGER"
        ");";
    rc = sqlite3_exec(db_, create_host_usage_sql, nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        CM_LOG_ERROR << "Failed to create host_usage table: " << errMsg << "\n";
        sqlite3_free(errMsg);
    }
}

void SQLiteDatabase::insertBatch(const std::string& container_name, const std::vector<ResourceSample>& samples) {
    std::lock_guard<std::mutex> lock(db_mutex);
    if (!db_ || samples.empty()) return;
    const char* sql = "INSERT INTO resource_samples (container_name, timestamp, cpu_usage, memory_usage, pids) VALUES (?, ?, ?, ?, ?);";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        for (const auto& sample : samples) {
            sqlite3_bind_text(stmt, 1, container_name.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_int64(stmt, 2, sample.timestamp);
            sqlite3_bind_double(stmt, 3, sample.cpu_usage);
            sqlite3_bind_int(stmt, 4, sample.memory_usage);
            sqlite3_bind_int(stmt, 5, sample.pids);
            sqlite3_step(stmt);
            sqlite3_reset(stmt);
        }
        sqlite3_finalize(stmt);
    }
}

// Extensible export: one CSV per table, in export_dir
void SQLiteDatabase::exportAllTablesToCSV(const std::string& export_dir) {
    std::lock_guard<std::mutex> lock(db_mutex);
    std::filesystem::create_directories(export_dir);

    // Export resource_samples table
    {
        std::string filename = export_dir + "/container_metrics.csv";
        std::ofstream file(filename);
        if (!file.is_open()) {
            CM_LOG_ERROR << "Failed to open resource_samples.csv for export: " << filename << "\n";
        } else {
            file << "container_name,timestamp,cpu_usage,memory_usage,pids\n";
            const char* sql = "SELECT container_name, timestamp, cpu_usage, memory_usage, pids FROM resource_samples;";
            sqlite3_stmt* stmt;
            if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) == SQLITE_OK) {
                while (sqlite3_step(stmt) == SQLITE_ROW) {
                    file << reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)) << ",";
                    file << sqlite3_column_int64(stmt, 1) << ",";
                    file << sqlite3_column_double(stmt, 2) << ",";
                    file << sqlite3_column_int(stmt, 3) << ",";
                    file << sqlite3_column_int(stmt, 4) << "\n";
                }
                sqlite3_finalize(stmt);
            }
            file.close();
        }
    }

    // Export host_usage table
    {
        std::string filename = export_dir + "/host_usage.csv";
        std::ofstream file(filename);
        if (!file.is_open()) {
            CM_LOG_ERROR << "Failed to open host_usage.csv for export: " << filename << "\n";
        } else {
            file << "timestamp,cpu_usage,memory_usage_mb\n";
            const char* sql = "SELECT timestamp, cpu_usage, memory_usage_mb FROM host_usage;";
            sqlite3_stmt* stmt;
            if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) == SQLITE_OK) {
                while (sqlite3_step(stmt) == SQLITE_ROW) {
                    file << sqlite3_column_int64(stmt, 0) << ",";
                    file << sqlite3_column_double(stmt, 1) << ",";
                    file << sqlite3_column_int64(stmt, 2) << "\n";
                }
                sqlite3_finalize(stmt);
            }
            file.close();
        }
    }
}

void SQLiteDatabase::saveHostUsage(int64_t timestamp_ms, double cpu_usage, uint64_t mem_usage_mb) {
    if (!db_) return;
    const char* sql = "INSERT INTO host_usage (timestamp, cpu_usage, memory_usage_mb) VALUES (?, ?, ?);";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_int64(stmt, 1, timestamp_ms);
        sqlite3_bind_double(stmt, 2, cpu_usage);
        sqlite3_bind_int64(stmt, 3, mem_usage_mb);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    }
}