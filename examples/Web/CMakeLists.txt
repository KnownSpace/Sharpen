cmake_minimum_required(VERSION 3.14.0)

add_executable(WebServer "${PROJECT_SOURCE_DIR}/examples/Web/WebServer.cpp")

target_link_libraries(WebServer sharpen)