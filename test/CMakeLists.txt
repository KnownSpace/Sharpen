cmake_minimum_required(VERSION 3.14.0)

include_directories("${PROJECT_SOURCE_DIR}/include")

#executables
#for benchmark
add_executable(webserver "${PROJECT_SOURCE_DIR}/test/WebServer.cpp")
#await test
add_executable(awaittest "${PROJECT_SOURCE_DIR}/test/AwaitTest.cpp")
#timer test
add_executable(timertest "${PROJECT_SOURCE_DIR}/test/TimerTest.cpp")
#console test
add_executable(consoletest "${PROJECT_SOURCE_DIR}/test/ConsoleTest.cpp")
#parallel test
add_executable(paralleltest "${PROJECT_SOURCE_DIR}/test/ParallelTest.cpp")
#template test
add_executable(templatetest "${PROJECT_SOURCE_DIR}/test/TemplateTest.cpp")
#link
target_link_libraries(webserver sharpen)

target_link_libraries(awaittest sharpen)
target_link_libraries(timertest sharpen)
target_link_libraries(consoletest sharpen)
target_link_libraries(paralleltest sharpen)
target_link_libraries(templatetest sharpen)
#test
enable_testing()
#tests
add_test(NAME await_test COMMAND ./awaittest)
add_test(NAME timer_test COMMAND ./timertest)
add_test(NAME console_test COMMAND ./consoletest)
add_test(NAME parallel_test COMMAND ./paralleltest)
add_test(NAME template_test COMMAND ./templatetest)
