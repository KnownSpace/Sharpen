cmake_minimum_required(VERSION 3.14.0)

add_executable(Encoder "${PROJECT_SOURCE_DIR}/examples/BitEncoder/Encoder.cpp")

target_link_libraries(Encoder sharpen)