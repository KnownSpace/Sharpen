
file(GLOB_RECURSE raft_status_map_src "${RAFT_TEST_DIR}/LogTest" "*.h" "*.hpp" "*.cpp" "*.cc")

include_directories("${COMMON_INCLUDE_DIR}")

add_executable(RaftLogTest ${raft_status_map_src})

target_link_libraries(RaftLogTest CommonTestLib)

target_link_libraries(RaftLogTest sharpen)

add_test(NAME RaftLogTest COMMAND "./RaftLogTest${extname}")