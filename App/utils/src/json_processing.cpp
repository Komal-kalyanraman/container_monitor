#include "json_processing.hpp"
#include <nlohmann/json.hpp>

bool parseContainerEvent(const std::string& json_str, ContainerEventInfo& info) {
    try {
        auto j = nlohmann::json::parse(json_str);
        if (j.contains("Type") && j["Type"] == "container") {
            info.status = j.value("status", j.value("Action", ""));
            info.id = j.value("id", j["Actor"].value("ID", ""));
            info.name = j["Actor"]["Attributes"].value("name", "");
            info.timeNano = j.value("timeNano", 0LL);
            return true;
        }
    } catch (...) {
        // Optionally log error
    }
    return false;
}