/**
 * @file json_processing.hpp
 * @brief Declares functions and structs for parsing container event JSON.
 */

#pragma once
#include <string>

/**
 * @struct ContainerEventInfo
 * @brief Holds parsed information from a container event.
 */
struct ContainerEventInfo {
    std::string name;        ///< Container name.
    std::string id;          ///< Container ID.
    std::string status;      ///< Event status (e.g., create, destroy).
    long long timeNano;      ///< Event timestamp in nanoseconds.
    std::string cpus;        ///< CPU limit (cores).
    std::string memory;      ///< Memory limit (MB).
    std::string pids_limit;  ///< PIDs limit.
};

/**
 * @brief Parses a container event JSON string into a ContainerEventInfo struct.
 * @param json_str JSON string representing the event.
 * @param info Reference to ContainerEventInfo to populate.
 * @return True if parsing was successful, false otherwise.
 */
bool parseContainerEvent(const std::string& json_str, ContainerEventInfo& info);