#include "event_processor.hpp"
#include <iostream>

EventProcessor::EventProcessor(EventQueue& queue, std::atomic<bool>& shutdown_flag)
    : queue_(queue), shutdown_flag_(shutdown_flag) {}

EventProcessor::~EventProcessor() { stop(); }

void EventProcessor::start() {
    running_ = true;
    worker_ = std::thread(&EventProcessor::processLoop, this);
}

void EventProcessor::stop() {
    running_ = false;
    queue_.shutdown();
    if (worker_.joinable()) worker_.join();
}

void EventProcessor::processLoop() {
    std::string event;
    while (running_ && !shutdown_flag_) {
        if (queue_.pop(event)) {
            // TODO: Parse event, print, or save to DB
            std::cout << "Processed event: " << event << std::endl;
        }
    }
}