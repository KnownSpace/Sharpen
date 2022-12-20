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
#template test
add_executable(templatetest "${PROJECT_SOURCE_DIR}/test/TemplateTest.cpp")
#file test
add_executable(filetest "${PROJECT_SOURCE_DIR}/test/FileTest.cpp")
#network test
add_executable(networktest "${PROJECT_SOURCE_DIR}/test/NetworkTest.cpp")
#option test
add_executable(optiontest "${PROJECT_SOURCE_DIR}/test/OptionTest.cpp")
#dummy type test
add_executable(dummytypetest "${PROJECT_SOURCE_DIR}/test/DummyTypeTest.cpp")
#quorum test
add_executable(quorumtest "${PROJECT_SOURCE_DIR}/test/QuorumTest.cpp")
#checksum test
add_executable(checksumtest "${PROJECT_SOURCE_DIR}/test/ChecksumTest.cpp")
#base64 test
add_executable(base64test "${PROJECT_SOURCE_DIR}/test/Base64Test.cpp")
#varint test
add_executable(varinttest "${PROJECT_SOURCE_DIR}/test/VarintTest.cpp")
#buffer compare test
add_executable(buffercomparetest "${PROJECT_SOURCE_DIR}/test/BufferCompareTest.cpp")
#int cast test
add_executable(intcasttest "${PROJECT_SOURCE_DIR}/test/IntCastTest.cpp")
#dns test
add_executable(dnstest "${PROJECT_SOURCE_DIR}/test/DnsTest.cpp")
#io_uring test
if(UNIX)
    add_executable(iouringtest "${PROJECT_SOURCE_DIR}/test/IoUringTest.cpp")
endif()
#iterator test
add_executable(iteratortest "${PROJECT_SOURCE_DIR}/test/IteratorTest.cpp")
#serializable test
add_executable(serializabletest "${PROJECT_SOURCE_DIR}/test/SerializableTest.cpp")
#functional test
add_executable(functionaltest "${PROJECT_SOURCE_DIR}/test/FunctionalTest.cpp")
target_link_libraries(templatetest sharpen)
target_link_libraries(filetest sharpen)
target_link_libraries(networktest sharpen)
target_link_libraries(optiontest sharpen)
target_link_libraries(dummytypetest sharpen)
target_link_libraries(quorumtest sharpen)
target_link_libraries(checksumtest sharpen)
target_link_libraries(base64test sharpen)
target_link_libraries(varinttest sharpen)
target_link_libraries(buffercomparetest sharpen)
target_link_libraries(intcasttest sharpen)
target_link_libraries(dnstest sharpen)
if(UNIX)
    target_link_libraries(iouringtest sharpen)
endif()
target_link_libraries(iteratortest sharpen)
target_link_libraries(serializabletest sharpen)
target_link_libraries(functionaltest sharpen)
#test

#tests
add_test(NAME template_test COMMAND "./templatetest${extname}")
add_test(NAME file_test COMMAND "./filetest${extname}")
add_test(NAME network_test COMMAND "./networktest${extname}")
add_test(NAME option_test COMMAND "./optiontest${extname}")
add_test(NAME dummy_type_test COMMAND "./dummytypetest${extname}")
add_test(NAME quorum_test COMMAND "./quorumtest${extname}")
add_test(NAME checksum_test COMMAND "./checksumtest${extname}")
add_test(NAME base64_test COMMAND "./base64test${extname}")
add_test(NAME varint_test COMMAND "./varinttest${extname}")
add_test(NAME buffercompare_test COMMAND "./buffercomparetest${extname}")
add_test(NAME intcast_test COMMAND "./intcasttest${extname}")
add_test(NAME dns_test COMMAND "./dnstest${extname}")
if(UNIX)
    add_test(NAME iouring_test COMMAND "./iouringtest${extname}")
endif()
add_test(NAME iterator_test COMMAND "./iteratortest${extname}")
add_test(NAME serializable_test COMMAND "./serializabletest${extname}")
add_test(NAME functional_test COMMAND "./functionaltest${extname}")