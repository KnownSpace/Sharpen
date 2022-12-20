cmake_minimum_required(VERSION 3.14.0)
project(sharpen VERSION 1.0.1)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED on)

include(CTest)
enable_testing()

set(SHARPEN_INCLUDE_DIRS "${PROJECT_SOURCE_DIR}/include/")

add_subdirectory("${PROJECT_SOURCE_DIR}/src")
add_subdirectory("${PROJECT_SOURCE_DIR}/test")

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
