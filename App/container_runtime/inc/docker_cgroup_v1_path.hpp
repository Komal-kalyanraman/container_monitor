#pragma once
#include "container_runtime_factory_interface.hpp"

class DockerCgroupV1PathFactory : public IContainerRuntimePathFactory {
public:
    ContainerResourcePaths getPaths(const std::string& container_id) const override {
        ContainerResourcePaths paths;
        std::string base = "docker/" + container_id;
        paths.cpu_path = "/sys/fs/cgroup/cpu/" + base + "/cpuacct.usage";
        paths.memory_path = "/sys/fs/cgroup/memory/" + base + "/memory.usage_in_bytes";
        paths.pids_path = "/sys/fs/cgroup/pids/" + base + "/pids.current";
        return paths;
    }
};