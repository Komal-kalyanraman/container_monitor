set(APP_NAME metrics_analyzer)

add_library(${APP_NAME} STATIC
    src/metrics_reader.cpp
)

target_include_directories(${APP_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/inc
)

target_link_libraries(${APP_NAME} PUBLIC glog::glog utils)
set_target_properties(${APP_NAME} PROPERTIES CXX_STANDARD 17)