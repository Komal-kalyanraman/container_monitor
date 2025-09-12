/**
 * @file sqlite_database.hpp
 * @brief Declares the SQLiteDatabase class for SQLite-based database operations.
 */

#pragma once
#include "database_interface.hpp"
#include <map>
#include <mutex>
#include <vector>
#include <string>
#include <sqlite3.h>
#include "common.hpp"

/**
 * @class SQLiteDatabase
 * @brief SQLite implementation of the IDatabaseInterface.
 *
 * Manages container and host usage data using SQLite, supports batch inserts,
 * schema setup, CSV export, and thread-safe access.
 */
class SQLiteDatabase : public IDatabaseInterface {
public:
    /**
     * @brief Constructs a SQLiteDatabase and opens the database file.
     * @param db_path Path to the SQLite database file.
     */
    SQLiteDatabase(const std::string& db_path);

    /**
     * @brief Destructor. Closes the database connection.
     */
    ~SQLiteDatabase();

    void saveContainer(const std::string& name, const ContainerInfo& info) override;
    void removeContainer(const std::string& name) override;
    void clearAll() override;
    ContainerInfo getContainer(const std::string& name) const override;
    size_t size() const override;
    const std::map<std::string, ContainerInfo>& getAll() const override;
    void setupSchema() override;
    void insertBatch(const std::string& container_name, const std::vector<ContainerMetrics>& metrics_vec) override;
    void exportAllTablesToCSV(const std::string& export_dir) override;
    void saveHostUsage(int64_t timestamp_ms, double cpu_usage_percent, double mem_usage_percent) override;

private:
    sqlite3* db_;                                   ///< SQLite database handle.
    mutable std::mutex db_mutex;                    ///< Mutex for thread-safe access.
    mutable std::map<std::string, ContainerInfo> cache_; ///< In-memory cache of container info.

    /**
     * @brief Loads container info cache from the database.
     */
    void loadCache() const;
};