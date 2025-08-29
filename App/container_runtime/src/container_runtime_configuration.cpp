#include "container_runtime_configuration.hpp"
#include "docker_cgroup_v1_path.hpp"
#include <memory>
#include <string>

std::unique_ptr<IContainerRuntimePathFactory> createPathFactory(const std::string& runtime, const std::string& cgroup_version) {
    if (runtime == "docker" && cgroup_version == "v1") {
        return std::make_unique<DockerCgroupV1PathFactory>();
    }
    // Add more combinations as needed
    return std::make_unique<DockerCgroupV1PathFactory>();
}