cmake_minimum_required(VERSION 3.15.0)

set(SIMPLETEST_INCLUDE_DIRS "${PROJECT_SOURCE_DIR}/lib/simpletest/include/")

include_directories("${SHARPEN_INCLUDE_DIRS}" "${SIMPLETEST_INCLUDE_DIRS}")

enable_testing()

if(WIN32)
    set(extname ".exe")
else()
    set(extname "")
endif()

set(TEST_DIR "${PROJECT_SOURCE_DIR}/test")

set(COMMON_INCLUDE_DIR "${TEST_DIR}/Common/include")

add_subdirectory("${TEST_DIR}/Common")

add_subdirectory("${TEST_DIR}/CoTest")

add_subdirectory("${TEST_DIR}/FsTest")

add_subdirectory("${TEST_DIR}/BufferTest")

add_subdirectory("${TEST_DIR}/NetTest")

add_subdirectory("${TEST_DIR}/ProcTest")

add_subdirectory("${TEST_DIR}/ActorTest")

add_subdirectory("${TEST_DIR}/RaftTest")