cmake_minimum_required(VERSION 3.14)
project(ContainerMonitor VERSION 0.0.1 LANGUAGES CXX)

# Set C++ standard and compiler options
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_compile_options(-Wno-deprecated-declarations)

# Find required packages (always required)
find_package(SQLite3 REQUIRED)
find_package(Curses REQUIRED)
find_package(glog REQUIRED)
find_package(nlohmann_json REQUIRED)

# Add subdirectories for modules
add_subdirectory(analysis)
add_subdirectory(ui)
add_subdirectory(kernel)
add_subdirectory(database)
add_subdirectory(utils)
add_subdirectory(cgroup)
add_subdirectory(runtime)

# Main executable
add_executable(container_monitor main.cpp)

# Link module libraries (replace with actual target names from submodules)
target_link_libraries(container_monitor
    analysis
    ui
    kernel
    database
    utils
    cgroup
    runtime
    SQLite::SQLite3
    ${CURSES_LIBRARIES}
    glog::glog
)

target_include_directories(container_monitor PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/analysis/inc
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/inc
    ${CMAKE_CURRENT_SOURCE_DIR}/kernel/inc
    ${CMAKE_CURRENT_SOURCE_DIR}/database/inc
    ${CMAKE_CURRENT_SOURCE_DIR}/utils/inc
    ${CMAKE_CURRENT_SOURCE_DIR}/cgroup/inc
    ${CMAKE_CURRENT_SOURCE_DIR}/runtime/inc
)

# Optionally, add thirdparty includes if needed
# include_directories(thirdparty/nlohmann_json/include)