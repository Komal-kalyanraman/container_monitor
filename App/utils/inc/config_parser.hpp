/**
 * @file config_parser.hpp
 * @brief Declares the ConfigParser class for parsing application configuration files.
 */

#pragma once
#include <string>
#include <string_view>
#include <unordered_map>
#include "common.hpp"

/**
 * @class ConfigParser
 * @brief Parses key-value configuration files and provides access to configuration parameters.
 *
 * Supports retrieval of string, integer, double, and boolean values, and conversion to MonitorConfig.
 */
class ConfigParser {
public:
    /**
     * @brief Loads configuration parameters from a file.
     * @param filename Path to the configuration file.
     * @return True if loading was successful, false otherwise.
     */
    bool load(std::string_view filename);

    /**
     * @brief Gets a string value for a given key.
     * @param key Configuration key.
     * @param default_val Default value if key is not found.
     * @return Value as string.
     */
    std::string get(std::string_view key, std::string_view default_val = "") const;

    /**
     * @brief Gets an integer value for a given key.
     * @param key Configuration key.
     * @param default_val Default value if key is not found or conversion fails.
     * @return Value as int.
     */
    int getInt(std::string_view key, int default_val = 0) const;

    /**
     * @brief Gets a double value for a given key.
     * @param key Configuration key.
     * @param default_val Default value if key is not found or conversion fails.
     * @return Value as double.
     */
    double getDouble(std::string_view key, double default_val = 0.0) const;

    /**
     * @brief Gets a boolean value for a given key.
     * @param key Configuration key.
     * @param default_val Default value if key is not found or conversion fails.
     * @return Value as bool.
     */
    bool getBool(std::string_view key, bool default_val = false) const;

    /**
     * @brief Converts loaded parameters to a MonitorConfig struct.
     * @return MonitorConfig object.
     */
    MonitorConfig toMonitorConfig() const;

    /**
     * @brief Prints the loaded configuration to the log.
     * @param cfg MonitorConfig object to print.
     */
    void printConfig(const MonitorConfig& cfg) const;

private:
    std::unordered_map<std::string, std::string> params_; ///< Map of configuration parameters.
};