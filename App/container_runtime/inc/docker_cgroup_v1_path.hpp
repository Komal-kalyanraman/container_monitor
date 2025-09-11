#pragma once
#include <cstdio>
#include "common.hpp"
#include "container_runtime_factory_interface.hpp"

class DockerCgroupV1PathFactory : public IContainerRuntimePathFactory {
public:
    ContainerResourcePaths getPaths(const std::string& container_id) const override {
        ContainerResourcePaths paths;
        char buf[CGROUP_PATH_BUF_SIZE];

        std::snprintf(buf, sizeof(buf), CGROUP_V1_CPU_PATH_FMT, container_id.c_str());
        paths.cpu_path = buf;
        std::snprintf(buf, sizeof(buf), CGROUP_V1_MEMORY_PATH_FMT, container_id.c_str());
        paths.memory_path = buf;
        std::snprintf(buf, sizeof(buf), CGROUP_V1_PIDS_PATH_FMT, container_id.c_str());
        paths.pids_path = buf;

        return paths;
    }
};