/**
 * @file sqlite_database.cpp
 * @brief Implements the SQLiteDatabase class for SQLite-based database operations.
 */

#include "sqlite_database.hpp"
#include <iostream>
#include <fstream>
#include <filesystem>
#include "logger.hpp"

/**
 * @brief Constructs a SQLiteDatabase and opens the database file.
 * @param db_path Path to the SQLite database file.
 */
SQLiteDatabase::SQLiteDatabase(const std::string& db_path) : db_(nullptr) {
    if (sqlite3_open(db_path.c_str(), &db_) != SQLITE_OK) {
        CM_LOG_ERROR << "Failed to open SQLite database: " << db_path << "\n";
        db_ = nullptr;
    }
}

/**
 * @brief Destructor. Closes the database connection.
 */
SQLiteDatabase::~SQLiteDatabase() {
    if (db_) sqlite3_close(db_);
}

/**
 * @brief Saves container information to the database and cache.
 * @param name Container name.
 * @param info ContainerInfo struct.
 */
void SQLiteDatabase::saveContainer(const std::string& name, const ContainerInfo& info) {
    if (!db_) return;
    const char* sql = SQL_INSERT_OR_REPLACE_CONTAINER;
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, info.id.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_double(stmt, 3, info.cpu_limit);
        sqlite3_bind_int(stmt, 4, info.memory_limit);
        sqlite3_bind_int(stmt, 5, info.pid_limit);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    }
    cache_[name] = info;
}

/**
 * @brief Retrieves container information by name from the cache.
 * @param name Container name.
 * @return ContainerInfo struct.
 */
ContainerInfo SQLiteDatabase::getContainer(const std::string& name) const {
    if (!db_) return {};
    loadCache();
    auto it = cache_.find(name);
    if (it != cache_.end()) return it->second;
    return {};
}

/**
 * @brief Returns the number of containers in the cache.
 * @return Number of containers.
 */
size_t SQLiteDatabase::size() const {
    loadCache();
    return cache_.size();
}

/**
 * @brief Returns all container information from the cache.
 * @return Map of container name to ContainerInfo.
 */
const std::map<std::string, ContainerInfo>& SQLiteDatabase::getAll() const {
    loadCache();
    return cache_;
}

/**
 * @brief Loads container info cache from the database.
 */
void SQLiteDatabase::loadCache() const {
    if (!db_) return;
    cache_.clear();
    const char* sql = SQL_SELECT_CONTAINER;
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            std::string name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
            ContainerInfo info;
            info.id = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
            info.cpu_limit = sqlite3_column_double(stmt, 2);
            info.memory_limit = sqlite3_column_int(stmt, 3);
            info.pid_limit = sqlite3_column_int(stmt, 4);
            cache_[name] = info;
        }
        sqlite3_finalize(stmt);
    }
}

/**
 * @brief Removes a container from the database and cache.
 * @param name Container name.
 */
void SQLiteDatabase::removeContainer(const std::string& name) {
    if (!db_) return;
    const char* sql = SQL_DELETE_CONTAINER_BY_NAME;
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    }
    cache_.erase(name);
}

/**
 * @brief Clears all tables and cached data in the database.
 */
void SQLiteDatabase::clearAll() {
    if (!db_) return;
    const char* sql1 = SQL_DELETE_ALL_CONTAINERS;
    const char* sql2 = SQL_DELETE_CONTAINER_METRICS;
    const char* sql3 = SQL_DELETE_HOST_USAGE;
    char* err_msg = nullptr;
    if (sqlite3_exec(db_, sql1, nullptr, nullptr, &err_msg) != SQLITE_OK) {
        CM_LOG_ERROR << "Failed to clear containers table: " << (err_msg ? err_msg : "unknown error") << "\n";
        sqlite3_free(err_msg);
    }
    if (sqlite3_exec(db_, sql2, nullptr, nullptr, &err_msg) != SQLITE_OK) {
        CM_LOG_ERROR << "Failed to clear container_metrics table: " << (err_msg ? err_msg : "unknown error") << "\n";
        sqlite3_free(err_msg);
    }
    if (sqlite3_exec(db_, sql3, nullptr, nullptr, &err_msg) != SQLITE_OK) {
        CM_LOG_ERROR << "Failed to clear host_usage table: " << (err_msg ? err_msg : "unknown error") << "\n";
        sqlite3_free(err_msg);
    }
    cache_.clear();
}

/**
 * @brief Sets up the database schema (tables).
 */
void SQLiteDatabase::setupSchema() {
    // Create containers table
    const char* create_containers_sql = SQL_CREATE_CONTAINERS_TABLE;
    char* errMsg = nullptr;
    int rc = sqlite3_exec(db_, create_containers_sql, nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        CM_LOG_ERROR << "Failed to create containers table: " << errMsg << "\n";
        sqlite3_free(errMsg);
    }

    // Create container_metrics table
    const char* create_container_metrics_sql = SQL_CREATE_CONTAINER_METRICS_TABLE;
    rc = sqlite3_exec(db_, create_container_metrics_sql, nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        CM_LOG_ERROR << "Failed to create container_metrics table: " << errMsg << "\n";
        sqlite3_free(errMsg);
    }

    // Create host_usage table
    const char* create_host_usage_sql = SQL_CREATE_HOST_USAGE_TABLE;
    rc = sqlite3_exec(db_, create_host_usage_sql, nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        CM_LOG_ERROR << "Failed to create host_usage table: " << errMsg << "\n";
        sqlite3_free(errMsg);
    }
}

/**
 * @brief Inserts a batch of metrics for a container.
 * @param container_name Container name.
 * @param metrics_vec Vector of ContainerMetrics.
 */
void SQLiteDatabase::insertBatch(const std::string& container_name, const std::vector<ContainerMetrics>& metrics_vec) {
    std::lock_guard<std::mutex> lock(db_mutex);
    if (!db_ || metrics_vec.empty()) return;
    const char* sql = SQL_INSERT_CONTAINER_METRICS;
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        for (const auto& metrics : metrics_vec) {
            sqlite3_bind_text(stmt, 1, container_name.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_int64(stmt, 2, metrics.timestamp);
            sqlite3_bind_double(stmt, 3, metrics.cpu_usage_percent);
            sqlite3_bind_double(stmt, 4, metrics.memory_usage_percent);
            sqlite3_bind_double(stmt, 5, metrics.pids_percent);
            sqlite3_step(stmt);
            sqlite3_reset(stmt);
        }
        sqlite3_finalize(stmt);
    }
}

/**
 * @brief Exports all tables to CSV files in the specified directory.
 * @param export_dir Directory to export CSV files.
 */
void SQLiteDatabase::exportAllTablesToCSV(const std::string& export_dir) {
    std::lock_guard<std::mutex> lock(db_mutex);
    std::filesystem::create_directories(export_dir);

    // Export container_metrics table
    {
        std::string filename = export_dir + CSV_CONTAINER_METRICS_FILENAME;
        std::ofstream file(filename);
        if (!file.is_open()) {
            CM_LOG_ERROR << "Failed to open container_metrics.csv for export: " << filename << "\n";
        } else {
            file << CSV_CONTAINER_METRICS_HEADER;
            const char* sql = SQL_SELECT_CONTAINER_METRICS;
            sqlite3_stmt* stmt;
            if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) == SQLITE_OK) {
                while (sqlite3_step(stmt) == SQLITE_ROW) {
                    file << reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)) << ",";
                    file << sqlite3_column_int64(stmt, 1) << ",";
                    file << sqlite3_column_double(stmt, 2) << ",";
                    file << sqlite3_column_double(stmt, 3) << ",";
                    file << sqlite3_column_double(stmt, 4) << "\n";
                }
                sqlite3_finalize(stmt);
            } else {
                CM_LOG_ERROR << "Failed to prepare export SQL for container_metrics: " << sqlite3_errmsg(db_) << "\n";
            }
            file.close();
        }
    }

    // Export host_usage table
    {
        std::string filename = export_dir + CSV_HOST_USAGE_FILENAME;
        std::ofstream file(filename);
        if (!file.is_open()) {
            CM_LOG_ERROR << "Failed to open host_usage.csv for export: " << filename << "\n";
        } else {
            file << CSV_HOST_USAGE_HEADER;
            const char* sql = SQL_SELECT_HOST_USAGE;
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

/**
 * @brief Saves host usage metrics to the database.
 * @param timestamp_ms Timestamp in milliseconds.
 * @param cpu_usage_percent CPU usage percent.
 * @param mem_usage_percent Memory usage percent.
 */
void SQLiteDatabase::saveHostUsage(int64_t timestamp_ms, double cpu_usage_percent, double mem_usage_percent) {
    if (!db_) return;
    const char* sql = SQL_INSERT_HOST_USAGE;
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_int64(stmt, 1, timestamp_ms);
        sqlite3_bind_double(stmt, 2, cpu_usage_percent);
        sqlite3_bind_double(stmt, 3, mem_usage_percent);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    }
}