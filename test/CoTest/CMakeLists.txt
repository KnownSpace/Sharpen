cmake_minimum_required(VERSION 3.15.0)

add_executable(AsyncTest "${TEST_DIR}/CoTest/AsyncTest.cpp")

target_link_libraries(AsyncTest sharpen)

add_test(NAME Async_Test COMMAND "./AsyncTest${extname}")

add_executable(TimerTest "${TEST_DIR}/CoTest/TimerTest.cpp")

target_link_libraries(TimerTest sharpen)

add_test(NAME Timer_Test COMMAND "./TimerTest${extname}")