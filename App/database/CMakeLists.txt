set(APP_NAME database)

add_library(${APP_NAME} STATIC
    src/database_interface.cpp
    src/sqlite_database.cpp
)

target_include_directories(${APP_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/inc
)

target_link_libraries(${APP_NAME} PUBLIC glog::glog SQLite::SQLite3 utils)
set_target_properties(${APP_NAME} PROPERTIES CXX_STANDARD 17)