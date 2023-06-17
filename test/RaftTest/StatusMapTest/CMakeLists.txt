
file(GLOB_RECURSE raft_status_map_src "${RAFT_TEST_DIR}/StatusMapTest" "*.h" "*.hpp" "*.cpp" "*.cc")

include_directories("${COMMON_INCLUDE_DIR}")

add_executable(RaftStatusMapTest ${raft_status_map_src})

target_link_libraries(RaftStatusMapTest CommonTestLib)

target_link_libraries(RaftStatusMapTest sharpen)

add_test(NAME RaftStatusMapTest COMMAND "./RaftStatusMapTest${extname}")