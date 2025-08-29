set(APP_NAME monitoring_service)

add_library(${APP_NAME} STATIC
    src/event_queue.cpp
    src/event_processor.cpp
    src/event_listener.cpp
    src/resource_monitor.cpp
    src/resource_thread_pool.cpp
)

target_include_directories(${APP_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/inc
    ${CMAKE_SOURCE_DIR}/database/inc
    ${CMAKE_SOURCE_DIR}/utils/inc
    ${CMAKE_SOURCE_DIR}/container_runtime/inc
)

target_link_libraries(${APP_NAME} PUBLIC utils glog::glog)
set_target_properties(${APP_NAME} PROPERTIES CXX_STANDARD 17)