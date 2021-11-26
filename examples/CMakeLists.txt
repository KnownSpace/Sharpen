cmake_minimum_required(VERSION 3.14.0)

include_directories("${PROJECT_SOURCE_DIR}/include")

add_subdirectory("${PROJECT_SOURCE_DIR}/examples/Restful")

add_subdirectory("${PROJECT_SOURCE_DIR}/examples/Web")

add_subdirectory("${PROJECT_SOURCE_DIR}/examples/MicroRpc")

add_subdirectory("${PROJECT_SOURCE_DIR}/examples/CommandLines")