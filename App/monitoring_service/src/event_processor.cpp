#include "event_processor.hpp"
#include <iostream>
#include "json_processing.hpp"

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
            try {
                ContainerEventInfo info;
                if (parseContainerEvent(event, info)) {
                    std::cout << "[Container Event] "
                              << "Name: " << info.name
                              << ", ID: " << info.id
                              << ", Status: " << info.status
                              << ", Time (ns): " << info.timeNano
                              << std::endl;
                }
            } catch (const std::exception& e) {
                std::cerr << "Event processing error: " << e.what() << std::endl;
            } catch (...) {
                std::cerr << "Unknown error during event processing." << std::endl;
            }
        }
    }
}