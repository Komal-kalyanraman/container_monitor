set(APP_NAME ui)

add_library(${APP_NAME} STATIC
    src/monitor_dashboard.cpp
)

target_include_directories(${APP_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/inc
)

target_link_libraries(${APP_NAME} PUBLIC glog::glog ${CURSES_LIBRARIES} utils)
set_target_properties(${APP_NAME} PROPERTIES CXX_STANDARD 17)