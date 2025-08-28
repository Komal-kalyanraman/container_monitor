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
#include "common.hpp" // For ResourceSample

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
    std::atomic<bool>& shutdown_flag_;
    IDatabaseInterface& db_;
    const MonitorConfig& cfg_;
    int batch_size_;
    int resource_sampling_interval_ms_;
    std::vector<std::thread> threads_;
    std::vector<std::vector<std::string>> thread_containers_;
    std::unordered_map<std::string, int> container_to_thread_;
    std::vector<std::map<std::string, std::vector<ResourceSample>>> thread_buffers_;
    std::mutex assign_mutex_;
    std::condition_variable cv_;
    bool running_ = false;
};