cmake_minimum_required(VERSION 3.15.0)

set(RAFT_TEST_DIR "${TEST_DIR}/RaftTest")

add_subdirectory("${RAFT_TEST_DIR}/StatusMapTest")

add_subdirectory("${RAFT_TEST_DIR}/LogTest")