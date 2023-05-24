cmake_minimum_required(VERSION 3.15.0)

add_executable(NetworkTest "${TEST_DIR}/NetTest/NetworkTest.cpp")

target_link_libraries(NetworkTest sharpen)

add_test(NAME Network_Test COMMAND "./NetworkTest${extname}")

add_executable(DnsTest "${TEST_DIR}/NetTest/DnsTest.cpp")

target_link_libraries(DnsTest sharpen)

add_test(NAME Dns_Test COMMAND "./DnsTest${extname}")