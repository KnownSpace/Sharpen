cmake_minimum_required(VERSION 3.15.0)

file(GLOB_RECURSE raft_append_src "${RAFT_TEST_DIR}/AppendTest" "*.h" "*.hpp" "*.cpp" "*.cc")

include_directories("${COMMON_INCLUDE_DIR}")

add_executable(RaftAppendTest ${raft_append_src})

target_link_libraries(RaftAppendTest CommonTestLib)

target_link_libraries(RaftAppendTest sharpen)

add_test(NAME RaftAppendTest COMMAND "./RaftAppendTest${extname}")