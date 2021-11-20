cmake_minimum_required(VERSION 3.14.0)

include_directories("${PROJECT_SOURCE_DIR}/include")

add_executable(MicroEncodeBenchmark "${PROJECT_SOURCE_DIR}/examples/MicroRpc/EncodeBenchmark.cpp")
add_executable(MicroDecodeBenchmark "${PROJECT_SOURCE_DIR}/examples/MicroRpc/DecodeBenchmark.cpp")

target_link_libraries(MicroEncodeBenchmark sharpen)
target_link_libraries(MicroDecodeBenchmark sharpen)