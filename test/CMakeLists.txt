cmake_minimum_required(VERSION 3.14.0)

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

#executables
#file test
add_executable(filetest "${PROJECT_SOURCE_DIR}/test/FileTest.cpp")
#network test
add_executable(networktest "${PROJECT_SOURCE_DIR}/test/NetworkTest.cpp")
#checksum test
add_executable(checksumtest "${PROJECT_SOURCE_DIR}/test/ChecksumTest.cpp")
#base64 test
add_executable(base64test "${PROJECT_SOURCE_DIR}/test/Base64Test.cpp")
#buffer compare test
add_executable(buffercomparetest "${PROJECT_SOURCE_DIR}/test/BufferCompareTest.cpp")
#dns test
add_executable(dnstest "${PROJECT_SOURCE_DIR}/test/DnsTest.cpp")
#io_uring test
if(UNIX)
    add_executable(iouringtest "${PROJECT_SOURCE_DIR}/test/IoUringTest.cpp")
endif()
#serializable test
add_executable(serializabletest "${PROJECT_SOURCE_DIR}/test/SerializableTest.cpp")
target_link_libraries(filetest sharpen)
target_link_libraries(networktest sharpen)
target_link_libraries(checksumtest sharpen)
target_link_libraries(base64test sharpen)
target_link_libraries(buffercomparetest sharpen)
target_link_libraries(dnstest sharpen)
if(UNIX)
    target_link_libraries(iouringtest sharpen)
endif()
target_link_libraries(serializabletest sharpen)
#test

#tests
add_test(NAME file_test COMMAND "./filetest${extname}")
add_test(NAME network_test COMMAND "./networktest${extname}")
add_test(NAME quorum_test COMMAND "./quorumtest${extname}")
add_test(NAME checksum_test COMMAND "./checksumtest${extname}")
add_test(NAME base64_test COMMAND "./base64test${extname}")
add_test(NAME buffercompare_test COMMAND "./buffercomparetest${extname}")
add_test(NAME dns_test COMMAND "./dnstest${extname}")
if(UNIX)
    add_test(NAME iouring_test COMMAND "./iouringtest${extname}")
endif()
add_test(NAME serializable_test COMMAND "./serializabletest${extname}")