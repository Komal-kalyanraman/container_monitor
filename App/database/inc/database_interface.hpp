/**
 * @file database_interface.hpp
 * @brief Declares the abstract interface for database operations in container monitoring.
 */

#pragma once
#include <vector>
#include <map>
#include <string>
#include "common.hpp"

/**
 * @class IDatabaseInterface
 * @brief Abstract interface for database operations.
 *
 * Provides methods for saving, retrieving, and managing container and host usage data.
 */
class IDatabaseInterface {
public:
    virtual ~IDatabaseInterface() = default;

    /**
     * @brief Save container information.
     * @param name Container name.
     * @param info ContainerInfo struct.
     */
    virtual void saveContainer(const std::string& name, const ContainerInfo& info) = 0;

    /**
     * @brief Retrieve container information by name.
     * @param name Container name.
     * @return ContainerInfo struct.
     */
    virtual ContainerInfo getContainer(const std::string& name) const = 0;

    /**
     * @brief Insert a batch of metrics for a container.
     * @param container_name Container name.
     * @param metrics_vec Vector of ContainerMetrics.
     */
    virtual void insertBatch(const std::string& container_name, const std::vector<ContainerMetrics>& metrics_vec) = 0;

    /**
     * @brief Remove a container by name.
     * @param name Container name.
     */
    virtual void removeContainer(const std::string& name) = 0;

    /**
     * @brief Clear all tables and cached data.
     */
    virtual void clearAll() = 0;

    /**
     * @brief Get the number of containers.
     * @return Number of containers.
     */
    virtual size_t size() const = 0;

    /**
     * @brief Get all container information.
     * @return Map of container name to ContainerInfo.
     */
    virtual const std::map<std::string, ContainerInfo>& getAll() const = 0;

    /**
     * @brief Setup database schema (tables).
     */
    virtual void setupSchema() = 0;

    /**
     * @brief Export all tables to CSV files.
     * @param export_dir Directory to export CSV files.
     */
    virtual void exportAllTablesToCSV(const std::string& export_dir) = 0;

    /**
     * @brief Save host usage metrics.
     * @param timestamp_ms Timestamp in milliseconds.
     * @param cpu_usage_percent CPU usage percent.
     * @param mem_usage_percent Memory usage percent.
     */
    virtual void saveHostUsage(int64_t timestamp_ms, double cpu_usage_percent, double mem_usage_percent) = 0;
};