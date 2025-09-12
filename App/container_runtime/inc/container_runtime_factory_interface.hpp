/**
 * @file container_runtime_factory_interface.hpp
 * @brief Declares the interface for container runtime path factories.
 */

#pragma once
#include <string>
#include "common.hpp"

/**
 * @class IContainerRuntimePathFactory
 * @brief Interface for container runtime resource path factories.
 *
 * Provides a method to obtain resource file paths for a given container ID.
 */
class IContainerRuntimePathFactory {
public:
    virtual ~IContainerRuntimePathFactory() = default;

    /**
     * @brief Returns resource file paths for the specified container ID.
     * @param container_id The container identifier.
     * @return ContainerResourcePaths Struct containing CPU, memory, and pids paths.
     */
    virtual ContainerResourcePaths getPaths(const std::string& container_id) const = 0;
};