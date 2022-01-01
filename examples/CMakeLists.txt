cmake_minimum_required(VERSION 3.14.0)

include_directories("${SHARPEN_INCLUDE_DIRS}")

add_subdirectory("${PROJECT_SOURCE_DIR}/examples/Restful")

add_subdirectory("${PROJECT_SOURCE_DIR}/examples/Web")

add_subdirectory("${PROJECT_SOURCE_DIR}/examples/MicroRpc")

add_subdirectory("${PROJECT_SOURCE_DIR}/examples/CommandLines")

add_subdirectory("${PROJECT_SOURCE_DIR}/examples/Tcp")

add_subdirectory("${PROJECT_SOURCE_DIR}/examples/Files")