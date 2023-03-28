cmake_minimum_required(VERSION 3.15.0)

set(SIMPLETEST_INCLUDE_DIRS "${PROJECT_SOURCE_DIR}/lib/simpletest/include/")

include_directories("${SHARPEN_INCLUDE_DIRS}" "${SIMPLETEST_INCLUDE_DIRS}")

enable_testing()

if(WIN32)
    set(extname ".exe")
else()
    set(extname "")
endif()

#Async Test
add_executable(AsyncTest "${PROJECT_SOURCE_DIR}/test/AsyncTest.cpp")

target_link_libraries(AsyncTest sharpen)

add_test(NAME Async_Test COMMAND "./AsyncTest${extname}")

#Timer Test
add_executable(TimerTest "${PROJECT_SOURCE_DIR}/test/TimerTest.cpp")

target_link_libraries(TimerTest sharpen)

add_test(NAME Timer_Test COMMAND "./TimerTest${extname}")

#File Test
add_executable(FileTest "${PROJECT_SOURCE_DIR}/test/FileTest.cpp")

target_link_libraries(FileTest sharpen)

add_test(NAME File_Test COMMAND "./FileTest${extname}")

#Network Test
add_executable(NetworkTest "${PROJECT_SOURCE_DIR}/test/NetworkTest.cpp")

target_link_libraries(NetworkTest sharpen)

add_test(NAME Network_Test COMMAND "./NetworkTest${extname}")

#Chksum Test
add_executable(ChksumTest "${PROJECT_SOURCE_DIR}/test/ChecksumTest.cpp")

target_link_libraries(ChksumTest sharpen)

add_test(NAME Chksum_Test COMMAND "./ChksumTest${extname}")

#BufferCompare Test
add_executable(BufferCompareTest "${PROJECT_SOURCE_DIR}/test/BufferCompareTest.cpp")

target_link_libraries(BufferCompareTest sharpen)

add_test(NAME BufferCompare_Test COMMAND "./BufferCompareTest${extname}")

#Base64 Test
add_executable(Base64Test "${PROJECT_SOURCE_DIR}/test/Base64Test.cpp")

target_link_libraries(Base64Test sharpen)

add_test(NAME Base64_Test COMMAND "./Base64Test${extname}")

#Dns Test
add_executable(DnsTest "${PROJECT_SOURCE_DIR}/test/DnsTest.cpp")

target_link_libraries(DnsTest sharpen)

add_test(NAME Dns_Test COMMAND "./DnsTest${extname}")

#Serializable Test

add_executable(SerializableTest "${PROJECT_SOURCE_DIR}/test/SerializableTest.cpp")

target_link_libraries(SerializableTest sharpen)

add_test(NAME Serializable_Test COMMAND "./SerializableTest${extname}")

#StatusMap Test
add_executable(StatusMapTest "${PROJECT_SOURCE_DIR}/test/StatusMapTest.cpp")

target_link_libraries(StatusMapTest sharpen)

add_test(NAME StatusMap_Test COMMAND "./StatusMapTest${extname}")

#Signal Test
add_executable(SignalTest "${PROJECT_SOURCE_DIR}/test/SignalTest.cpp")

target_link_libraries(SignalTest sharpen)

add_test(NAME Signal_Test COMMAND "./SignalTest${extname}")

#Process Test
add_executable(ProcessTest "${PROJECT_SOURCE_DIR}/test/ProcessTest.cpp")

target_link_libraries(ProcessTest sharpen)

add_test(NAME Process_Test COMMAND "./ProcessTest${extname}")