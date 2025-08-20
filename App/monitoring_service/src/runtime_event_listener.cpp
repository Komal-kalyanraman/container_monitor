#include "runtime_event_listener.hpp"
#include "logger.hpp"
#include <chrono>
#include <cstdio>

RuntimeEventListener::RuntimeEventListener(const MonitorConfig& config, ContainerEventCallback callback, std::atomic<bool>& shutdown_flag)
    : config_(config), callback_(callback), shutdown_flag_(shutdown_flag), running_(false) {}

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

    while (running_ && !shutdown_flag_) {
        FILE* pipe = popen(runtime_cmd.c_str(), "r");
        if (!pipe) {
            CM_LOG_ERROR << "Failed to start event command\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(config_.container_event_refresh_interval_ms));
            continue;
        }

        char buffer[4096];
        while (fgets(buffer, sizeof(buffer), pipe) != nullptr && running_ && !shutdown_flag_) {
            std::string event_json(buffer);
            if (callback_) {
                callback_(event_json);
            }
        }
        pclose(pipe);

        std::this_thread::sleep_for(std::chrono::milliseconds(config_.container_event_refresh_interval_ms));
    }
}