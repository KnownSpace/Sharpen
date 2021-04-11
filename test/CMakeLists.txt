cmake_minimum_required(VERSION 3.8.0)

include_directories("${PROJECT_SOURCE_DIR}/include")
aux_source_directory("${PROJECT_SOURCE_DIR}/test" src)
add_executable(mytest ${src})
target_link_libraries(mytest sharpen)
#test
enable_testing()
add_test(NAME basic COMMAND mytest)
add_test(NAME mutex COMMAND mytest mutex)
add_test(NAME workpool COMMAND mytest workpool)
add_test(NAME file COMMAND mytest file)
