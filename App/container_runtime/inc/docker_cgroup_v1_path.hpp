/**
 * @file docker_cgroup_v1_path.hpp
 * @brief Declares the DockerCgroupV1PathFactory for Docker cgroup v1 resource paths.
 */

#pragma once
#include <cstdio>
#include "common.hpp"
#include "container_runtime_factory_interface.hpp"

/**
 * @class DockerCgroupV1PathFactory
 * @brief Factory for generating Docker cgroup v1 resource file paths.
 *
 * Implements IContainerRuntimePathFactory to provide CPU, memory, and pids paths
 * for a given container ID using cgroup v1 conventions.
 */
class DockerCgroupV1PathFactory : public IContainerRuntimePathFactory {
public:
    /**
     * @brief Returns resource file paths for the specified container ID.
     * @param container_id The container identifier.
     * @return ContainerResourcePaths Struct containing CPU, memory, and pids paths.
     */
    ContainerResourcePaths getPaths(const std::string& container_id) const override {
        ContainerResourcePaths paths;
        char buf[CGROUP_PATH_BUF_SIZE];

        std::snprintf(buf, sizeof(buf), DOCKER_CGROUP_V1_CPU_PATH_FMT, container_id.c_str());
        paths.cpu_path = buf;
        std::snprintf(buf, sizeof(buf), DOCKER_CGROUP_V1_MEMORY_PATH_FMT, container_id.c_str());
        paths.memory_path = buf;
        std::snprintf(buf, sizeof(buf), DOCKER_CGROUP_V1_PIDS_PATH_FMT, container_id.c_str());
        paths.pids_path = buf;

        return paths;
    }
};