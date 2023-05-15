cmake_minimum_required(VERSION 3.15.0)

add_executable(ProcessTest "${TEST_DIR}/ProcTest/ProcessTest.cpp")

target_link_libraries(ProcessTest sharpen)

add_test(NAME Process_Test COMMAND "./ProcessTest${extname}")

add_executable(SignalTest "${TEST_DIR}/ProcTest/SignalTest.cpp")

target_link_libraries(SignalTest sharpen)

add_test(NAME Signal_Test COMMAND "./SignalTest${extname}")