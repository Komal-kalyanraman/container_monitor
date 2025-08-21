#include "event_listener.hpp"
#include "logger.hpp"
#include <chrono>
#include <cstdio>

RuntimeEventListener::RuntimeEventListener(const MonitorConfig& config, EventQueue& queue, std::atomic<bool>& shutdown_flag)
    : config_(config), event_queue_(queue), shutdown_flag_(shutdown_flag), running_(false) {}

RuntimeEventListener::~RuntimeEventListener() {
    stop();
}

void RuntimeEventListener::start() {
    running_ = true;
    event_thread_ = std::thread(&RuntimeEventListener::eventThreadFunc, this);
}

void RuntimeEventListener::stop() {
    running_ = false;
    if (event_thread_.joinable()) {
        event_thread_.join();
    }
}

void RuntimeEventListener::eventThreadFunc() {
    std::string runtime_cmd;
    if (config_.runtime == "docker") {
        runtime_cmd = "docker events --format '{{json .}}' --since 0m";
    } else if (config_.runtime == "podman") {
        runtime_cmd = "podman events --format '{{json .}}' --since 0m";
    } else {
        CM_LOG_ERROR << "Unsupported container runtime: " << config_.runtime << "\n";
        return;
    }

    FILE* pipe = popen(runtime_cmd.c_str(), "r");
    if (!pipe) {
        CM_LOG_ERROR << "Failed to start event command\n";
        return;
    }

    char buffer[4096];
    while (running_ && !shutdown_flag_) {
        if (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
            std::string event_json(buffer);
            event_queue_.push(event_json); // Push event to queue
        } else {
            break;
        }
    }
    pclose(pipe);
}