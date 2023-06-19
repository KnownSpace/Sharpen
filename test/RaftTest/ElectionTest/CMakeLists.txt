cmake_minimum_required(VERSION 3.15.0)

file(GLOB_RECURSE raft_election_src "${RAFT_TEST_DIR}/ElectionTest" "*.h" "*.hpp" "*.cpp" "*.cc")

include_directories("${COMMON_INCLUDE_DIR}")

add_executable(RaftElectionTest ${raft_election_src})

target_link_libraries(RaftElectionTest CommonTestLib)

target_link_libraries(RaftElectionTest sharpen)

add_test(NAME RaftElectionTest COMMAND "./RaftElectionTest${extname}")