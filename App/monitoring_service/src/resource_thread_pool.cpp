/**
 * @file resource_thread_pool.cpp
 * @brief Implements the ResourceThreadPool class for parallel container resource monitoring.
 */

#include "resource_thread_pool.hpp"
#include <map>
#include <cmath> 
#include <mutex>
#include <vector>
#include <chrono>
#include <cstring>
#include <mqueue.h>
#include <algorithm>
#include <unordered_map>
#include "common.hpp"
#include "logger.hpp"
#include "metrics_reader.hpp"
#include "docker_cgroup_v1_path.hpp"

std::mutex cout_mutex;

/**
 * @brief Constructs a ResourceThreadPool.
 * @param cfg Monitor configuration.
 * @param shutdown_flag Reference to the application's shutdown flag.
 * @param db Reference to the database interface.
 */
ResourceThreadPool::ResourceThreadPool(const MonitorConfig& cfg, std::atomic<bool>& shutdown_flag, IDatabaseInterface& db)
    : cfg_(cfg), shutdown_flag_(shutdown_flag), db_(db), thread_containers_(cfg.thread_count),
      thread_buffers_(cfg.thread_count), thread_local_paths_(cfg.thread_count), thread_local_info_(cfg.thread_count)
{
    // Initialize the factory once
    if (cfg_.runtime == "docker" && cfg_.cgroup == "v1") {
        pathFactory_ = std::make_unique<DockerCgroupV1PathFactory>();
    }
}

/**
 * @brief Destructor. Ensures all threads are stopped and buffers flushed.
 */
ResourceThreadPool::~ResourceThreadPool() {
    stop();
}

/**
 * @brief Starts all worker threads.
 */
void ResourceThreadPool::start() {
    running_ = true;
    for (int i = 0; i < cfg_.thread_count; ++i) {
        threads_.emplace_back([this, i]() { workerLoop(i); });
    }
}

/**
 * @brief Stops all worker threads and flushes buffers.
 */
void ResourceThreadPool::stop() {
    running_ = false;
    cv_.notify_all();
    flushAllBuffers();
    for (auto& t : threads_) {
        if (t.joinable()) t.join();
    }
}

/**
 * @brief Adds a container to the thread pool for monitoring.
 * @param name Container name.
 */
void ResourceThreadPool::addContainer(const std::string& name) {
    std::unique_lock<std::mutex> lock(assign_mutex_);
    flushAllBuffers();
    int min_thread = -1, min_load = cfg_.thread_capacity + 1;
    for (int i = 0; i < cfg_.thread_count; ++i) {
        if (thread_containers_[i].size() < cfg_.thread_capacity && thread_containers_[i].size() < min_load) {
            min_thread = i;
            min_load = thread_containers_[i].size();
        }
    }
    if (min_thread == -1) {
        CM_LOG_INFO << "[ThreadPool] Capacity full, cannot assign container: " << name << "\n";
        return;
    }
    thread_containers_[min_thread].push_back(name);
    container_to_thread_[name] = min_thread;

    // Fetch full container info from database
    ContainerInfo info = db_.getContainer(name);
    thread_local_info_[min_thread][name] = info;
    ContainerResourcePaths paths = pathFactory_->getPaths(info.id);
    thread_local_paths_[min_thread][name] = paths;

    CM_LOG_INFO << "[ThreadPool] Paths for container " << name << ":\n"
                << "  CPU: " << paths.cpu_path << "\n"
                << "  Memory: " << paths.memory_path << "\n"
                << "  PIDs: " << paths.pids_path << "\n";
    
    CM_LOG_INFO << "[ThreadPool] Assigned container " << name << " to thread " << min_thread << "\n";
    cv_.notify_all();
}

/**
 * @brief Removes a container from the thread pool.
 * @param name Container name.
 */
void ResourceThreadPool::removeContainer(const std::string& name) {
    std::unique_lock<std::mutex> lock(assign_mutex_);
    flushAllBuffers();
    auto it = container_to_thread_.find(name);
    if (it != container_to_thread_.end()) {
        int thread_idx = it->second;
        auto& vec = thread_containers_[thread_idx];
        vec.erase(std::remove(vec.begin(), vec.end(), name), vec.end());
        container_to_thread_.erase(it);
        thread_local_info_[thread_idx].erase(name);
        thread_local_paths_[thread_idx].erase(name);
        CM_LOG_INFO << "[ThreadPool] Removed container " << name << " from thread " << thread_idx << "\n";
        cv_.notify_all();
    }
}

/**
 * @brief Flushes all metric buffers to the database.
 */
void ResourceThreadPool::flushAllBuffers() {
    for (int i = 0; i < cfg_.thread_count; ++i) {
        auto& buffers = thread_buffers_[i];
        for (auto& [name, buffer] : buffers) {
            if (!buffer.empty()) db_.insertBatch(name, buffer);
            buffer.clear();
        }
    }
}

/**
 * @brief Gets the current thread-to-container assignments.
 * @return Map of thread index to vector of container names.
 */
std::map<int, std::vector<std::string>> ResourceThreadPool::getAssignments() {
    std::unique_lock<std::mutex> lock(assign_mutex_);
    std::map<int, std::vector<std::string>> result;
    for (int i = 0; i < cfg_.thread_count; ++i) {
        result[i] = thread_containers_[i];
    }
    return result;
}

/**
 * @brief Worker thread function for collecting metrics.
 * @param thread_index Index of the worker thread.
 *
 * - Collects metrics for assigned containers.
 * - Batches metrics and sends max values to the UI via message queue.
 * - Inserts batches into the database.
 * - Waits for the configured sampling interval.
 * - Handles shutdown and buffer flushing.
 */
void ResourceThreadPool::workerLoop(int thread_index) {
    auto& buffers = thread_buffers_[thread_index];
    auto& local_paths = thread_local_paths_[thread_index];

    // Print METRIC_MQ_MSG_SIZE for debugging
    CM_LOG_INFO << "[Thread " << thread_index << "] METRIC_MQ_MSG_SIZE: " << METRIC_MQ_MSG_SIZE << "\n";

    // Open message queue (shared, fixed size)
    mqd_t mq;
    struct mq_attr attr;
    attr.mq_flags = 0;
    attr.mq_maxmsg = METRIC_MQ_MAX_MSG;
    attr.mq_msgsize = METRIC_MQ_MSG_SIZE;
    attr.mq_curmsgs = 0;

    CM_LOG_INFO << "[Thread " << thread_index << "] Attempting mq_open with name: " << METRIC_MQ_NAME << "\n";
    CM_LOG_INFO << "[Thread " << thread_index << "] mq_attr: "
                << "mq_flags=" << attr.mq_flags
                << ", mq_maxmsg=" << attr.mq_maxmsg
                << ", mq_msgsize=" << attr.mq_msgsize
                << ", mq_curmsgs=" << attr.mq_curmsgs << "\n";

    mq = mq_open(METRIC_MQ_NAME.data(), O_RDWR | O_CREAT, 0644, &attr);

    if (mq == (mqd_t)-1) {
        CM_LOG_ERROR << "[Thread " << thread_index << "] Failed to open message queue: " << strerror(errno)
                     << " (errno=" << errno << ")" << "\n";
    } else {
        CM_LOG_INFO << "[Thread " << thread_index << "] Message queue opened successfully. \n";
    }

    while (running_ && !shutdown_flag_) {
        std::vector<std::string> containers;
        {
            std::unique_lock<std::mutex> lock(assign_mutex_);
            containers = thread_containers_[thread_index];
        }
        
        // Add this check right here:
        if (containers.empty()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_MS_MEDIUM)); // or 1000ms
            continue;
        }

        for (const auto& name : containers) {
            ContainerMetrics metrics;
            metrics.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch()).count();

            auto it = local_paths.find(name);
            if (it == local_paths.end()) continue;
            MetricsReader reader(it->second, 1);

            // Fetch container limits from local cache
            auto info_it = thread_local_info_[thread_index].find(name);
            if (info_it == thread_local_info_[thread_index].end()) continue;
            const ContainerInfo& info = info_it->second;

            // Memory and pids as percent
            metrics.memory_usage_percent = reader.getMemoryUsagePercent(info);
            metrics.pids_percent = reader.getPidsPercent(info);

            // CPU usage delta calculation
            uint64_t curr_cpu_ns = reader.readUintFromFile(it->second.cpu_path);
            auto prev_it = prev_cpu_usage_.find(name);
            metrics.cpu_usage_percent = ZERO_PERCENT;
            if (prev_it != prev_cpu_usage_.end()) {
                int64_t prev_ts = prev_it->second.first;
                uint64_t prev_ns = prev_it->second.second;
                int64_t delta_ms = metrics.timestamp - prev_ts;
                int64_t delta_ns = static_cast<int64_t>(curr_cpu_ns) - static_cast<int64_t>(prev_ns);
                if (delta_ms > 0 && delta_ns > 0 && info.cpu_limit > 0) {
                    double cpu_sec = (double)delta_ns / NANOSECONDS_PER_SECOND;
                    double interval_sec = (double)delta_ms / MILLISECONDS_PER_SECOND;
                    double percent = (cpu_sec / interval_sec) / info.cpu_limit * PERCENT_FACTOR;
                    metrics.cpu_usage_percent = std::round(percent * PERCENT_FACTOR) / PERCENT_FACTOR;
                } else {
                    metrics.cpu_usage_percent = ZERO_PERCENT;
                }
            }
            prev_cpu_usage_[name] = {metrics.timestamp, curr_cpu_ns};

            buffers[name].push_back(metrics);

            if (buffers[name].size() >= cfg_.batch_size) {
                if (cfg_.ui_enabled) { 
                    double max_cpu = ZERO_PERCENT;
                    double max_mem = ZERO_PERCENT;
                    double max_pids = ZERO_PERCENT;
                    for (const auto& m : buffers[name]) {
                        max_cpu = std::max(max_cpu, m.cpu_usage_percent);
                        max_mem = std::max(max_mem, m.memory_usage_percent);
                        max_pids = std::max(max_pids, m.pids_percent);
                    }

                // Prepare message
                    ContainerMaxMetricsMsg max_msg;
                    std::memset(&max_msg, 0, sizeof(max_msg));
                    max_msg.max_cpu_usage_percent = max_cpu;
                    max_msg.max_memory_usage_percent = max_mem;
                    max_msg.max_pids_percent = max_pids;
                    std::memset(max_msg.container_id, 0, sizeof(max_msg.container_id));
                    std::strncpy(max_msg.container_id, name.c_str(), sizeof(max_msg.container_id));

                    mq_send(mq, reinterpret_cast<const char*>(&max_msg), METRIC_MQ_MSG_SIZE, 0);
                }

                // Insert batch to DB and clear buffer
                db_.insertBatch(name, buffers[name]);
                buffers[name].clear();
            }
        }
        // Wait for per-container sampling time × number of containers
        int total_wait_ms = containers.size() * cfg_.resource_sampling_interval_ms;
        std::unique_lock<std::mutex> lock(assign_mutex_);
        cv_.wait_for(lock, std::chrono::milliseconds(total_wait_ms), [this]() { return !running_; });
    }

    mq_close(mq);
    
    // On shutdown, flush all buffers for this thread
    for (auto& [name, buffer] : buffers) {
        if (!buffer.empty()) db_.insertBatch(name, buffer);
    }
}