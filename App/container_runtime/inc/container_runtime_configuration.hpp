/**
 * @file container_runtime_configuration.hpp
 * @brief Declares the factory selector for container runtime path factories.
 */

#pragma once
#include <memory>
#include <string>
#include "container_runtime_factory_interface.hpp"

/**
 * @brief Selects and creates an appropriate container runtime path factory.
 *
 * Returns a unique pointer to an implementation of IContainerRuntimePathFactory
 * based on the specified runtime and cgroup version.
 *
 * @param runtime The container runtime name (e.g., "docker", "podman").
 * @param cgroup_version The cgroup version (e.g., "v1", "v2").
 * @return std::unique_ptr<IContainerRuntimePathFactory> Factory instance.
 */
std::unique_ptr<IContainerRuntimePathFactory> createPathFactory(const std::string& runtime, const std::string& cgroup_version);