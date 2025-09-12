/**
 * @file resource_thread_pool.hpp
 * @brief Declares the ResourceThreadPool class for parallel container resource monitoring.
 */

#pragma once
#include <vector>
#include <thread>
#include <atomic>
#include <map>
#include <mutex>
#include <string>
#include <unordered_map>
#include <condition_variable>
#include "database_interface.hpp" 
#include "common.hpp"
#include "container_runtime_factory_interface.hpp"

/**
 * @class ResourceThreadPool
 * @brief Manages a pool of threads for collecting container resource metrics in parallel.
 *
 * Assigns containers to threads, collects metrics, batches data for database insertion,
 * and sends max metrics to the UI via message queue.
 */
class ResourceThreadPool {
public:
    /**
     * @brief Constructs a ResourceThreadPool.
     * @param cfg Monitor configuration.
     * @param shutdown_flag Reference to the application's shutdown flag.
     * @param db Reference to the database interface.
     */
    ResourceThreadPool(const MonitorConfig& cfg, std::atomic<bool>& shutdown_flag, IDatabaseInterface& db);

    /**
     * @brief Destructor. Ensures all threads are stopped and buffers flushed.
     */
    ~ResourceThreadPool();

    /**
     * @brief Starts all worker threads.
     */
    void start();

    /**
     * @brief Stops all worker threads and flushes buffers.
     */
    void stop();

    /**
     * @brief Adds a container to the thread pool for monitoring.
     * @param name Container name.
     */
    void addContainer(const std::string& name);

    /**
     * @brief Removes a container from the thread pool.
     * @param name Container name.
     */
    void removeContainer(const std::string& name);

    /**
     * @brief Flushes all metric buffers to the database.
     */
    void flushAllBuffers();

    /**
     * @brief Gets the current thread-to-container assignments.
     * @return Map of thread index to vector of container names.
     */
    std::map<int, std::vector<std::string>> getAssignments();

private:
    /**
     * @brief Worker thread function for collecting metrics.
     * @param thread_index Index of the worker thread.
     */
    void workerLoop(int thread_index);

    std::atomic<bool>& shutdown_flag_;                ///< Reference to shutdown flag.
    bool running_ = false;                            ///< Indicates if the pool is running.
    IDatabaseInterface& db_;                          ///< Reference to database interface.
    std::mutex assign_mutex_;                         ///< Mutex for assignments and buffers.
    const MonitorConfig& cfg_;                        ///< Monitor configuration.
    std::condition_variable cv_;                      ///< Condition variable for thread coordination.
    std::vector<std::thread> threads_;                ///< Worker threads.
    std::vector<std::vector<std::string>> thread_containers_;   ///< Containers assigned to each thread.
    std::unordered_map<std::string, int> container_to_thread_;  ///< Container to thread index mapping.
    std::unique_ptr<IContainerRuntimePathFactory> pathFactory_; ///< Path factory for resource files.
    std::vector<std::map<std::string, ContainerInfo>> thread_local_info_;               ///< Per-thread container info.
    std::unordered_map<std::string, std::pair<int64_t, uint64_t>> prev_cpu_usage_;      ///< Previous CPU usage for delta calculation.
    std::vector<std::map<std::string, ContainerResourcePaths>> thread_local_paths_;     ///< Per-thread resource paths.
    std::vector<std::map<std::string, std::vector<ContainerMetrics>>> thread_buffers_;  ///< Per-thread metric buffers.
};