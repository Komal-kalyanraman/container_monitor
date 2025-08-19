#include "config_parser.hpp"
#include "common.hpp"
#include <fstream>
#include <sstream>

bool ConfigParser::load(std::string_view filename) {
    std::ifstream file{std::string(filename)};
    if (!file.is_open()) return false;
    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;
        auto pos = line.find('=');
        if (pos == std::string::npos) continue;
        std::string key = line.substr(0, pos);
        std::string val = line.substr(pos + 1);
        params_[key] = val;
    }
    return true;
}

std::string ConfigParser::get(std::string_view key, std::string_view default_val) const {
    auto it = params_.find(std::string(key));
    return it != params_.end() ? it->second : std::string(default_val);
}

int ConfigParser::getInt(std::string_view key, int default_val) const {
    auto val = get(key, "");
    try { return std::stoi(val); } catch (...) { return default_val; }
}

double ConfigParser::getDouble(std::string_view key, double default_val) const {
    auto val = get(key, "");
    try { return std::stod(val); } catch (...) { return default_val; }
}

bool ConfigParser::getBool(std::string_view key, bool default_val) const {
    auto val = get(key, "");
    if (val == "true" || val == "1") return true;
    if (val == "false" || val == "0") return false;
    return default_val;
}

MonitorConfig ConfigParser::toMonitorConfig() const {
    MonitorConfig cfg;
    cfg.runtime                 = get(KEY_RUNTIME, DEFAULT_RUNTIME);
    cfg.cgroup                  = get(KEY_CGROUP, DEFAULT_CGROUP);
    cfg.database                = get(KEY_DATABASE, DEFAULT_DATABASE);
    cfg.sampling_interval_ms    = getInt(KEY_SAMPLING_INTERVAL_MS, DEFAULT_SAMPLING_INTERVAL_MS);
    cfg.db_path                 = get(KEY_DB_PATH, DEFAULT_DB_PATH);
    cfg.ui_enabled              = getBool(KEY_UI_ENABLED, DEFAULT_UI_ENABLED);
    cfg.batch_size              = getInt(KEY_BATCH_SIZE, DEFAULT_BATCH_SIZE);
    cfg.alert_warning           = getDouble(KEY_ALERT_WARNING, DEFAULT_ALERT_WARNING);
    cfg.alert_critical          = getDouble(KEY_ALERT_CRITICAL, DEFAULT_ALERT_CRITICAL);
    cfg.alert_violation         = getDouble(KEY_ALERT_VIOLATION, DEFAULT_ALERT_VIOLATION);
    return cfg;
}