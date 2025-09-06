#pragma once
#include <memory>
#include <string>
#include "container_runtime_factory_interface.hpp"

// Factory selector function declaration
std::unique_ptr<IContainerRuntimePathFactory> createPathFactory(const std::string& runtime, const std::string& cgroup_version);