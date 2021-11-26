cmake_minimum_required(VERSION 3.14.0)

include_directories("${PROJECT_SOURCE_DIR}/include")

add_executable(Console "${PROJECT_SOURCE_DIR}/examples/CommandLines/Console.cpp")

target_link_libraries(Console sharpen)