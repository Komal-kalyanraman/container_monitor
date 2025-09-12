/**
 * @file container_runtime_configuration.cpp
 * @brief Implements the factory selector for container runtime path factories.
 */

#include "container_runtime_configuration.hpp"
#include "docker_cgroup_v1_path.hpp"
#include <memory>
#include <string>

/**
 * @brief Selects and creates an appropriate container runtime path factory.
 *
 * Currently supports Docker with cgroup v1. Extend this function to support
 * additional runtime and cgroup combinations as needed.
 *
 * @param runtime The container runtime name (e.g., "docker", "podman").
 * @param cgroup_version The cgroup version (e.g., "v1", "v2").
 * @return std::unique_ptr<IContainerRuntimePathFactory> Factory instance.
 */
std::unique_ptr<IContainerRuntimePathFactory> createPathFactory(const std::string& runtime, const std::string& cgroup_version) {
    if (runtime == "docker" && cgroup_version == "v1") {
        return std::make_unique<DockerCgroupV1PathFactory>();
    }
    // Add more combinations as needed
    return std::make_unique<DockerCgroupV1PathFactory>();
}