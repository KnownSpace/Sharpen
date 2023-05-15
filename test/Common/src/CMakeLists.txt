cmake_minimum_required(VERSION 3.15.0)

file(GLOB_RECURSE common_src "${TEST_DIR}/Common/src" "*.h" "*.hpp" "*.cpp" "*.cc")

include_directories("${COMMON_INCLUDE_DIR}")

add_library(CommonTestLib ${common_src})

target_link_libraries(CommonTestLib sharpen)