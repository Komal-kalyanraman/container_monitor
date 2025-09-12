set(APP_NAME utils)

add_library(${APP_NAME} STATIC
    src/initializer.cpp
    src/config_parser.cpp
    src/json_processing.cpp    
)

target_include_directories(${APP_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/inc
)

target_link_libraries(${APP_NAME} PUBLIC glog::glog)
set_target_properties(${APP_NAME} PROPERTIES CXX_STANDARD 17)