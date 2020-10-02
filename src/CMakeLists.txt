cmake_minimum_required(VERSION 3.8.0)

include_directories("${PROJECT_SOURCE_DIR}/include")
aux_source_directory("${PROJECT_SOURCE_DIR}/src" src)
add_library(sharpen ${src})