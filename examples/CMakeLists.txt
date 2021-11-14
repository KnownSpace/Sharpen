cmake_minimum_required(VERSION 3.14.0)

include_directories("${PROJECT_SOURCE_DIR}/include")

add_executable(WebServer "${PROJECT_SOURCE_DIR}/examples/WebServer.cpp")

add_executable(RestfulClient "${PROJECT_SOURCE_DIR}/examples/RestfulClient.cpp")

target_link_libraries(WebServer sharpen)

target_link_libraries(RestfulClient sharpen)