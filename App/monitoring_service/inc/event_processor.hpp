#pragma once
#include "event_queue.hpp"
#include "database_interface.hpp"
#include <atomic>
#include <thread>
#include <unordered_map>
#include <string>

class EventProcessor {
public:
    EventProcessor(EventQueue& queue, std::atomic<bool>& shutdown_flag, IDatabaseInterface& db);
    ~EventProcessor();
    void start();
    void stop();

private:
    void processLoop();
    EventQueue& queue_;
    std::atomic<bool>& shutdown_flag_;
    IDatabaseInterface& db_;
    std::thread worker_;
    bool running_ = false;
    std::unordered_map<std::string, std::string> name_to_id;
};