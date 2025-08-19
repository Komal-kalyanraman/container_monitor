#pragma once
#include <string>
#include <string_view>
#include <unordered_map>
#include "common.hpp"

class ConfigParser {
public:
    bool load(std::string_view filename);
    std::string get(std::string_view key, std::string_view default_val = "") const;
    int getInt(std::string_view key, int default_val = 0) const;
    double getDouble(std::string_view key, double default_val = 0.0) const;
    bool getBool(std::string_view key, bool default_val = false) const;
    MonitorConfig toMonitorConfig() const;
private:
    std::unordered_map<std::string, std::string> params_;
};