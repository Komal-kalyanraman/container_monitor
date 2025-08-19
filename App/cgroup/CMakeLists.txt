set(APP_NAME cgroup)

add_library(${APP_NAME} STATIC
    src/cgroup_v1_service.cpp
    src/cgroup_v2_service.cpp
    src/cgroup_factory.cpp
)

target_include_directories(${APP_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/inc
)

target_link_libraries(${APP_NAME} PUBLIC glog::glog utils)
set_target_properties(${APP_NAME} PROPERTIES CXX_STANDARD 17)