#pragma once
#include <string>

struct ContainerEventInfo {
    std::string name;
    std::string id;
    std::string status;
    long long timeNano;
};

bool parseContainerEvent(const std::string& json_str, ContainerEventInfo& info);