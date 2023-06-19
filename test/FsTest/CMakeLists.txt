cmake_minimum_required(VERSION 3.15.0)

add_executable(DirectoryTest "${TEST_DIR}/FsTest/DirectoryTest.cpp")

target_link_libraries(DirectoryTest sharpen)

add_test(NAME Directory_Test COMMAND "./DirectoryTest${extname}")

add_executable(FileTest "${TEST_DIR}/FsTest/FileTest.cpp")

target_link_libraries(FileTest sharpen)

add_test(NAME File_Test COMMAND "./FileTest${extname}")