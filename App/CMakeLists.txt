cmake_minimum_required(VERSION 3.14)
project(ContainerMonitor VERSION 0.1.1 LANGUAGES CXX)

# Set default build type to Release if not specified
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
endif()

# Set C++ standard and compiler options
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_compile_options(-Wno-deprecated-declarations)

# Set flags for Debug and Release
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -pg")
set(CMAKE_EXE_LINKER_FLAGS_DEBUG "-pg")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")
set(CMAKE_EXE_LINKER_FLAGS_RELEASE "")

# Find required packages (always required)
find_package(SQLite3 REQUIRED)
find_package(Curses REQUIRED)
find_package(Threads REQUIRED)
find_package(glog 0.4.0 REQUIRED)
find_package(nlohmann_json REQUIRED)

# Add subdirectories for modules
add_subdirectory(analysis)
add_subdirectory(ui)
add_subdirectory(metrics_analyzer)
add_subdirectory(database)
add_subdirectory(utils)
add_subdirectory(container_runtime)
add_subdirectory(monitoring_service)

# Main executable
add_executable(container_monitor main.cpp)

# Link module libraries (replace with actual target names from submodules)
target_link_libraries(container_monitor
    analysis
    ui
    metrics_analyzer
    database
    utils
    container_runtime
    monitoring_service
    SQLite::SQLite3
    ${CURSES_LIBRARIES}
    glog::glog
)

target_include_directories(container_monitor PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/analysis/inc
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/inc
    ${CMAKE_CURRENT_SOURCE_DIR}/metrics_analyzer/inc
    ${CMAKE_CURRENT_SOURCE_DIR}/database/inc
    ${CMAKE_CURRENT_SOURCE_DIR}/utils/inc
    ${CMAKE_CURRENT_SOURCE_DIR}/container_runtime/inc
    ${CMAKE_CURRENT_SOURCE_DIR}/monitoring_service/inc
)

# Optionally, add thirdparty includes if needed
# include_directories(thirdparty/nlohmann_json/include)