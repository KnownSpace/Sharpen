cmake_minimum_required(VERSION 3.15.0)

add_executable(Base64Test "${TEST_DIR}/BufferTest/Base64Test.cpp")

target_link_libraries(Base64Test sharpen)

add_test(NAME Base64_Test COMMAND "./Base64Test${extname}")

add_executable(BufferCompareTest "${TEST_DIR}/BufferTest/BufferCompareTest.cpp")

target_link_libraries(BufferCompareTest sharpen)

add_test(NAME BufferCompare_Test COMMAND "./BufferCompareTestextname")

add_executable(ChecksumTest "${TEST_DIR}/BufferTest/ChecksumTest.cpp")

target_link_libraries(ChecksumTest sharpen)

add_test(NAME Checksum_Test COMMAND "./ChecksumTest${extname}")

add_executable(SerializableTest "${TEST_DIR}/BufferTest/SerializableTest.cpp")

target_link_libraries(SerializableTest sharpen)

add_test(NAME Serializable_Test COMMAND "./SerializableTest${extname}")