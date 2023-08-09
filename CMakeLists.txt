cmake_minimum_required(VERSION 3.15.0)
project(sharpen VERSION 1.0.3)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED on)

include(CTest)
enable_testing()

set(SHARPEN_INCLUDE_DIRS "${PROJECT_SOURCE_DIR}/include/")

## Check WSL
if("${CMAKE_SYSTEM}" MATCHES "WSL")
    add_definitions(-DSHARPEN_ON_WSL)
endif()

add_subdirectory("${PROJECT_SOURCE_DIR}/src")
add_subdirectory("${PROJECT_SOURCE_DIR}/test")

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
