#include "live_metric_aggregator.hpp"
#include <iostream>
#include <mqueue.h>
#include <cstring>
#include <cerrno>
#include <thread>
#include "common.hpp"

LiveMetricAggregator::LiveMetricAggregator(std::atomic<bool>& shutdown_flag)
    : shutdown_flag_(shutdown_flag) {}

LiveMetricAggregator::~LiveMetricAggregator() {
    stop();
}

void LiveMetricAggregator::start() {
    if (!running_) {
        running_ = true;
        worker_ = std::thread(&LiveMetricAggregator::run, this);
    }
}

void LiveMetricAggregator::stop() {
    running_ = false;
    if (worker_.joinable()) {
        worker_.join();
    }
}

void LiveMetricAggregator::run() {
    std::cout << "[C++] Waiting for message queue '" << METRIC_MQ_NAME << "' to appear..." << std::endl;

    mqd_t mqd;
    int attempts = 0;
    while (attempts < 50 && !shutdown_flag_) {
        mqd = mq_open(METRIC_MQ_NAME.data(), O_RDONLY | O_NONBLOCK);
        if (mqd != (mqd_t)-1) {
            std::cout << "[C++] Message queue opened successfully on attempt " << (attempts + 1) << "." << std::endl;
            break;
        } else {
            std::cout << "[C++] Attempt " << (attempts + 1) << ": Queue not found, retrying..." << std::endl;
            attempts++;
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }
    if (mqd == (mqd_t)-1) {
        std::cout << "[C++] Message queue not found after waiting." << std::endl;
        return;
    }

    std::cout << "[C++] Waiting for messages..." << std::endl;
    ContainerMaxMetricsMsg msg;
    unsigned int prio;
    while (running_ && !shutdown_flag_) {
        ssize_t bytes = mq_receive(mqd, reinterpret_cast<char*>(&msg), METRIC_MQ_MSG_SIZE, &prio);
        if (bytes >= 0) {
            std::cout << "Container: " << std::string(msg.container_id)
                      << " | Max CPU: " << msg.max_cpu_usage_percent
                      << " | Max Mem: " << msg.max_memory_usage_percent
                      << " | Max PIDs: " << msg.max_pids_percent << std::endl;
        } else if (errno == EAGAIN) {
            // No message, just wait
        } else if (errno == EINTR) {
            break; // Interrupted by signal
        } else {
            std::cerr << "[C++] mq_receive error: " << strerror(errno) << std::endl;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    mq_close(mqd);
    std::cout << "[C++] Message queue closed." << std::endl;
}