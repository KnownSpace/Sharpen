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
#file test
add_executable(filetest "${PROJECT_SOURCE_DIR}/test/FileTest.cpp")
#network test
add_executable(networktest "${PROJECT_SOURCE_DIR}/test/NetworkTest.cpp")
#option test
add_executable(optiontest "${PROJECT_SOURCE_DIR}/test/OptionTest.cpp")
#link
target_link_libraries(webserver sharpen)

target_link_libraries(awaittest sharpen)
target_link_libraries(timertest sharpen)
target_link_libraries(consoletest sharpen)
target_link_libraries(paralleltest sharpen)
target_link_libraries(templatetest sharpen)
target_link_libraries(filetest sharpen)
target_link_libraries(networktest sharpen)
target_link_libraries(optiontest sharpen)
#test
enable_testing()
#tests
if(WIN32)
    set(extname ".exe")
else()
    set(extname "")
endif()
add_test(NAME await_test COMMAND "./awaittest${extname}")
add_test(NAME timer_test COMMAND "./timertest${extname}")
add_test(NAME console_test COMMAND "./consoletest${extname}")
add_test(NAME parallel_test COMMAND "./paralleltest${extname}")
add_test(NAME template_test COMMAND "./templatetest${extname}")
add_test(NAME file_test COMMAND "./filetest${extname}")
add_test(NAME network_test COMMAND "./networktest${extname}")
add_test(NAME option_test COMMAND "./optiontest${extname}")