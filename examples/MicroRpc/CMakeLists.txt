cmake_minimum_required(VERSION 3.14.0)

include_directories("${PROJECT_SOURCE_DIR}/include")

add_executable(MicroEncodeBenchmark "${PROJECT_SOURCE_DIR}/examples/MicroRpc/EncodeBenchmark.cpp")

target_link_libraries(MicroEncodeBenchmark sharpen)