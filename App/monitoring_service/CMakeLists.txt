set(APP_NAME monitoring_service)

add_library(${APP_NAME} STATIC
    src/event_queue.cpp
    src/event_processor.cpp
    src/event_listener.cpp
)

target_include_directories(${APP_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/inc
)

target_link_libraries(${APP_NAME} PUBLIC utils glog::glog)
set_target_properties(${APP_NAME} PROPERTIES CXX_STANDARD 17)