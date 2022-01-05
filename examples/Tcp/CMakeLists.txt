cmake_minimum_required(VERSION 3.14.0)

add_executable(SelfConnection "${PROJECT_SOURCE_DIR}/examples/Tcp/SelfConnection.cpp")
add_executable(DnsList "${PROJECT_SOURCE_DIR}/examples/Tcp/DnsList.cpp")

target_link_libraries(SelfConnection sharpen)
target_link_libraries(DnsList sharpen)