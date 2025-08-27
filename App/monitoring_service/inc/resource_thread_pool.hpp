#pragma once
#include <vector>
#include <thread>
#include <atomic>
#include <map>
#include <mutex>
#include <string>
#include <unordered_map>
#include <condition_variable>

class ResourceThreadPool {
public:
    ResourceThreadPool(int thread_count, int thread_capacity, std::atomic<bool>& shutdown_flag);
    ~ResourceThreadPool();

    void start();
    void stop();

    void addContainer(const std::string& name);
    void removeContainer(const std::string& name);

    std::map<int, std::vector<std::string>> getAssignments();

private:
    void workerLoop(int thread_index);

    int thread_count_;
    int thread_capacity_;
    std::atomic<bool>& shutdown_flag_;
    std::vector<std::thread> threads_;
    std::vector<std::vector<std::string>> thread_containers_;
    std::unordered_map<std::string, int> container_to_thread_;
    std::mutex assign_mutex_;
    std::condition_variable cv_;
    bool running_ = false;
};