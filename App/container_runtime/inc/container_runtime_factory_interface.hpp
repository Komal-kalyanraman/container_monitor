#pragma once
#include <string>
#include "common.hpp"

class IContainerRuntimePathFactory {
public:
    virtual ~IContainerRuntimePathFactory() = default;
    virtual ContainerResourcePaths getPaths(const std::string& container_id) const = 0;
};