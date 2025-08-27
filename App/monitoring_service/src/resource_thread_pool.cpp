#include "resource_thread_pool.hpp"
#include "logger.hpp"
#include <algorithm>

ResourceThreadPool::ResourceThreadPool(int thread_count, int thread_capacity, std::atomic<bool>& shutdown_flag)
    : thread_count_(thread_count), thread_capacity_(thread_capacity), shutdown_flag_(shutdown_flag),
      thread_containers_(thread_count) {}

ResourceThreadPool::~ResourceThreadPool() {
    stop();
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
    for (auto& t : threads_) {
        if (t.joinable()) t.join();
    }
}

void ResourceThreadPool::addContainer(const std::string& name) {
    std::unique_lock<std::mutex> lock(assign_mutex_);
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
    CM_LOG_INFO << "[ThreadPool] Assigned container " << name << " to thread " << min_thread << "\n";
    cv_.notify_all();
}

void ResourceThreadPool::removeContainer(const std::string& name) {
    std::unique_lock<std::mutex> lock(assign_mutex_);
    auto it = container_to_thread_.find(name);
    if (it != container_to_thread_.end()) {
        int thread_idx = it->second;
        auto& vec = thread_containers_[thread_idx];
        vec.erase(std::remove(vec.begin(), vec.end(), name), vec.end());
        container_to_thread_.erase(it);
        CM_LOG_INFO << "[ThreadPool] Removed container " << name << " from thread " << thread_idx << "\n";
        cv_.notify_all();
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
    while (running_ && !shutdown_flag_) {
        std::vector<std::string> containers;
        {
            std::unique_lock<std::mutex> lock(assign_mutex_);
            containers = thread_containers_[thread_index];
        }
        for (const auto& name : containers) {
            // TODO: Poll resource usage for container 'name'
            CM_LOG_INFO << "[Thread " << thread_index << "] Monitoring container: " << name << "\n";
        }
        std::unique_lock<std::mutex> lock(assign_mutex_);
        cv_.wait_for(lock, std::chrono::milliseconds(500), [this]() { return !running_; });
    }
}