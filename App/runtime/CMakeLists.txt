set(APP_NAME runtime)

add_library(${APP_NAME} STATIC
    src/docker_runtime.cpp
    src/podman_runtime.cpp
    src/runtime_command_factory.cpp
)

target_include_directories(${APP_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/inc
)

target_link_libraries(${APP_NAME} PUBLIC glog::glog utils)
set_target_properties(${APP_NAME} PROPERTIES CXX_STANDARD 17)