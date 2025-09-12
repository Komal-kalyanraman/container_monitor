#pragma once
#include "common.hpp"

class Initializer {
public:
    static void initLogger(int argc, char* argv[], const MonitorConfig& cfg);
    static void unlinkMessageQueue();
    static MonitorConfig parseConfig();
    static void setupSignalHandlers(void (*handler)(int));
};