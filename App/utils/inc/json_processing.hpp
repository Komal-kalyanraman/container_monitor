#pragma once
#include <string>

struct ContainerEventInfo {
    std::string name;
    std::string id;
    std::string status;
    long long timeNano;
    std::string cpus;
    std::string memory;
    std::string pids_limit;
};

bool parseContainerEvent(const std::string& json_str, ContainerEventInfo& info);