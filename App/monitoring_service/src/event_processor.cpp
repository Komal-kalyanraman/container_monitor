#include "event_processor.hpp"
#include <iostream>
#include "json_processing.hpp"

EventProcessor::EventProcessor(EventQueue& queue, std::atomic<bool>& shutdown_flag, IDatabaseInterface& db)
    : queue_(queue), shutdown_flag_(shutdown_flag), db_(db) {}

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
                            << ", Time (ns): " << info.timeNano;
                    if (info.status == "create") {
                        std::cout << ", CPUs: " << info.cpus
                                << ", Memory: " << info.memory
                                << ", PIDs limit: " << info.pids_limit;
                        double cpus = std::stod(info.cpus);
                        int memory = std::stoi(info.memory);
                        int pids_limit = std::stoi(info.pids_limit);
                        db_.saveContainer(info.name, std::make_tuple(info.id, cpus, memory, pids_limit));
                    } else if (info.status == "destroy") {
                        db_.removeContainer(info.name);
                        std::cout << " [Container Removed]";
                    }
                    std::cout << std::endl;
                }
            } catch (const std::exception& e) {
                std::cerr << "Event processing error: " << e.what() << std::endl;
            } catch (...) {
                std::cerr << "Unknown error during event processing." << std::endl;
            }
        }
    }
}