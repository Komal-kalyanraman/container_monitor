#include "resource_thread_pool.hpp"
#include <algorithm>
#include <map>
#include <vector>
#include <chrono>
#include "common.hpp"
#include "logger.hpp"
#include "docker_cgroup_v1_path.hpp"

ResourceThreadPool::ResourceThreadPool(const MonitorConfig& cfg, std::atomic<bool>& shutdown_flag, IDatabaseInterface& db)
    : cfg_(cfg), thread_count_(cfg.thread_count), thread_capacity_(cfg.thread_capacity),
      shutdown_flag_(shutdown_flag), db_(db),
      batch_size_(cfg.batch_size), resource_sampling_interval_ms_(cfg.resource_sampling_interval_ms),
      thread_containers_(cfg.thread_count), thread_buffers_(cfg.thread_count)
{
    // Initialize the factory once
    if (cfg_.runtime == "docker" && cfg_.cgroup == "v1") {
        pathFactory_ = std::make_unique<DockerCgroupV1PathFactory>();
    }
}

ResourceThreadPool::~ResourceThreadPool() {
    stop();
}

double ResourceThreadPool::getCpuUsage(const std::string& name) {
    std::unique_lock<std::mutex> map_lock(container_paths_mutex_);
    auto it = container_paths_.find(name);
    if (it == container_paths_.end()) return 0.0;
    return 0.0;
}

int ResourceThreadPool::getMemoryUsage(const std::string& name) {
    std::unique_lock<std::mutex> map_lock(container_paths_mutex_);
    auto it = container_paths_.find(name);
    if (it == container_paths_.end()) return 0;
    return 0;
}

int ResourceThreadPool::getPids(const std::string& name) {
    std::unique_lock<std::mutex> map_lock(container_paths_mutex_);
    auto it = container_paths_.find(name);
    if (it == container_paths_.end()) return 0;
    return 0;
}

void ResourceThreadPool::start() {
    running_ = true;
    for (int i = 0; i < thread_count_; ++i) {
        threads_.emplace_back([this, i]() { workerLoop(i); });
    }
}

void ResourceThreadPool::stop() {
    running_ = false;
    cv_.notify_all();
    flushAllBuffers();
    for (auto& t : threads_) {
        if (t.joinable()) t.join();
    }
}

void ResourceThreadPool::addContainer(const std::string& name) {
    std::unique_lock<std::mutex> lock(assign_mutex_);
    flushAllBuffers();
    int min_thread = -1, min_load = thread_capacity_ + 1;
    for (int i = 0; i < thread_count_; ++i) {
        if (thread_containers_[i].size() < thread_capacity_ && thread_containers_[i].size() < min_load) {
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
    std::unique_lock<std::mutex> map_lock(container_paths_mutex_);
    container_paths_[name] = pathFactory_->getPaths(name);

    const auto& paths = container_paths_[name];
    CM_LOG_INFO << "[ThreadPool] Paths for container " << name << ":\n"
                << "  CPU: " << paths.cpu_path << "\n"
                << "  Memory: " << paths.memory_path << "\n"
                << "  PIDs: " << paths.pids_path << "\n";
    
    CM_LOG_INFO << "[ThreadPool] Assigned container " << name << " to thread " << min_thread << "\n";
    cv_.notify_all();
}

void ResourceThreadPool::removeContainer(const std::string& name) {
    std::unique_lock<std::mutex> lock(assign_mutex_);
    flushAllBuffers();
    auto it = container_to_thread_.find(name);
    if (it != container_to_thread_.end()) {
        int thread_idx = it->second;
        auto& vec = thread_containers_[thread_idx];
        vec.erase(std::remove(vec.begin(), vec.end(), name), vec.end());
        container_to_thread_.erase(it);
        std::unique_lock<std::mutex> map_lock(container_paths_mutex_);
        container_paths_.erase(name);
        CM_LOG_INFO << "[ThreadPool] Removed container " << name << " from thread " << thread_idx << "\n";
        cv_.notify_all();
    }
}

void ResourceThreadPool::flushAllBuffers() {
    for (int i = 0; i < thread_count_; ++i) {
        auto& buffers = thread_buffers_[i];
        for (auto& [name, buffer] : buffers) {
            if (!buffer.empty()) db_.insertBatch(name, buffer);
            buffer.clear();
        }
    }
}

std::map<int, std::vector<std::string>> ResourceThreadPool::getAssignments() {
    std::unique_lock<std::mutex> lock(assign_mutex_);
    std::map<int, std::vector<std::string>> result;
    for (int i = 0; i < thread_count_; ++i) {
        result[i] = thread_containers_[i];
    }
    return result;
}

void ResourceThreadPool::workerLoop(int thread_index) {
    auto& buffers = thread_buffers_[thread_index];

    while (running_ && !shutdown_flag_) {
        std::vector<std::string> containers;
        {
            std::unique_lock<std::mutex> lock(assign_mutex_);
            containers = thread_containers_[thread_index];
        }
        for (const auto& name : containers) {
            ResourceSample sample;
            sample.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch()).count();
            sample.cpu_usage = getCpuUsage(name);
            sample.memory_usage = getMemoryUsage(name);
            sample.pids = getPids(name);

            buffers[name].push_back(sample);

            if (buffers[name].size() >= batch_size_) {
                db_.insertBatch(name, buffers[name]);
                buffers[name].clear();
                CM_LOG_INFO << "[Thread " << thread_index << "] Batch inserted for container: " << name << "\n";
            }
        }
        // Wait for per-container sampling time × number of containers
        int total_wait_ms = containers.size() * resource_sampling_interval_ms_;
        std::unique_lock<std::mutex> lock(assign_mutex_);
        cv_.wait_for(lock, std::chrono::milliseconds(total_wait_ms), [this]() { return !running_; });
    }
    // On shutdown, flush all buffers for this thread
    for (auto& [name, buffer] : buffers) {
        if (!buffer.empty()) db_.insertBatch(name, buffer);
    }
}