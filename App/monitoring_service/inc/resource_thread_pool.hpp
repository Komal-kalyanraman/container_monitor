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

class ResourceThreadPool {
public:
    ResourceThreadPool(const MonitorConfig& cfg, std::atomic<bool>& shutdown_flag, IDatabaseInterface& db);
    ~ResourceThreadPool();

    void start();
    void stop();

    void addContainer(const std::string& name);
    void removeContainer(const std::string& name);
    void flushAllBuffers();

    std::map<int, std::vector<std::string>> getAssignments();

private:
    void workerLoop(int thread_index);

    int thread_count_;
    int thread_capacity_;
    int container_count_ = 0;
    std::atomic<bool>& shutdown_flag_;
    IDatabaseInterface& db_;
    const MonitorConfig& cfg_;
    int batch_size_;
    int resource_sampling_interval_ms_;
    std::vector<std::thread> threads_;
    std::vector<std::vector<std::string>> thread_containers_;
    std::unordered_map<std::string, int> container_to_thread_;
    std::vector<std::map<std::string, std::vector<ContainerMetrics>>> thread_buffers_;
    std::mutex assign_mutex_;
    std::condition_variable cv_;
    std::unique_ptr<IContainerRuntimePathFactory> pathFactory_;
    std::vector<std::map<std::string, ContainerResourcePaths>> thread_local_paths_;
    std::vector<std::map<std::string, ContainerInfo>> thread_local_info_;
    bool running_ = false;
    bool ui_enabled_;
    std::unordered_map<std::string, std::pair<int64_t, uint64_t>> prev_cpu_usage_;
};