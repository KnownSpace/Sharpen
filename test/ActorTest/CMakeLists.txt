cmake_minimum_required(VERSION 3.15.0)

file(GLOB_RECURSE actor_src "${TEST_DIR}/ActorTest/" "*.h" "*.hpp" "*.cpp" "*.cc")

include_directories("${COMMON_INCLUDE_DIR}")

add_executable(ActorTest ${actor_src})

target_link_libraries(ActorTest CommonTestLib)

target_link_libraries(ActorTest sharpen)