#include "json_processing.hpp"
#include <nlohmann/json.hpp>
#include <iostream>
#include <cstdio>
#include <sstream>
#include "common.hpp"

bool getResourceConstraintsFromInspect(const std::string& container_id, ContainerEventInfo& info) {
    std::string cmd = "docker inspect " + container_id;
    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe) return false;

    std::stringstream ss;
    char buffer[4096];
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        ss << buffer;
    }
    pclose(pipe);

    try {
        auto j = nlohmann::json::parse(ss.str());
        if (!j.is_array() || j.empty()) return false;
        auto& inspect = j[0];
        auto& hostConfig = inspect["HostConfig"];
        // CPUs: Docker stores as NanoCpus (divide by 1e9 for cores)
        if (hostConfig.contains("NanoCpus")) {
            long long nano_cpus = hostConfig["NanoCpus"];
            info.cpus = std::to_string(nano_cpus / NANOSECONDS_PER_SECOND);
        }
        // Memory: bytes, convert to MB
        if (hostConfig.contains("Memory")) {
            long long mem_bytes = hostConfig["Memory"];
            info.memory = std::to_string(mem_bytes / (BYTES_PER_KILOBYTE * KILOBYTES_PER_MEGABYTE)) + "MB";
        }
        // PIDs limit
        if (hostConfig.contains("PidsLimit")) {
            info.pids_limit = std::to_string(hostConfig["PidsLimit"].get<int>());
        }
        return true;
    } catch (...) {
        // Optionally log error
    }
    return false;
}

bool parseContainerEvent(const std::string& json_str, ContainerEventInfo& info) {
    try {
        auto j = nlohmann::json::parse(json_str);
        if (j.contains("Type") && j["Type"] == "container") {
            info.status = j.value("status", j.value("Action", ""));
            info.id = j.value("id", j["Actor"].value("ID", ""));
            info.name = j["Actor"]["Attributes"].value("name", "");
            info.timeNano = j.value("timeNano", 0LL);

            // Only extract resource constraints for "create" event
            if (info.status == "create") {
                auto& attrs = j["Actor"]["Attributes"];
                info.cpus = attrs.value("cpus", "");
                info.memory = attrs.value("memory", "");
                info.pids_limit = attrs.value("pids-limit", "");
                // If missing, fetch from docker inspect
                if (info.cpus.empty() || info.memory.empty() || info.pids_limit.empty()) {
                    getResourceConstraintsFromInspect(info.id, info);
                }
            }
            return true;
        }
    } catch (...) {
        // Optionally log error
    }
    return false;
}