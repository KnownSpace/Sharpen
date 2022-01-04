cmake_minimum_required(VERSION 3.14.0)

include_directories("${SHARPEN_INCLUDE_DIRS}")

#executables
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
#arena test
add_executable(arenatest "${PROJECT_SOURCE_DIR}/test/ArenaTest.cpp")
#dummy type test
add_executable(dummytypetest "${PROJECT_SOURCE_DIR}/test/DummyTypeTest.cpp")
#micro rpc test
add_executable(microrpctest "${PROJECT_SOURCE_DIR}/test/MicroRpcTest.cpp")
#quorum test
add_executable(quorumtest "${PROJECT_SOURCE_DIR}/test/QuorumTest.cpp")
#checksum test
add_executable(checksumtest "${PROJECT_SOURCE_DIR}/test/ChecksumTest.cpp")
#bloom filter test
add_executable(bloomfiltertest "${PROJECT_SOURCE_DIR}/test/BloomFilterTest.cpp")
#base64 test
add_executable(base64test "${PROJECT_SOURCE_DIR}/test/Base64Test.cpp")
#memory table test
add_executable(memorytabletest "${PROJECT_SOURCE_DIR}/test/MemoryTableTest.cpp")
#varint test
add_executable(varinttest "${PROJECT_SOURCE_DIR}/test/VarintTest.cpp")
#buffer compare test
add_executable(buffercomparetest "${PROJECT_SOURCE_DIR}/test/BufferCompareTest.cpp")
#int cast test
add_executable(intcasttest "${PROJECT_SOURCE_DIR}/test/IntCastTest.cpp")
#sst test
add_executable(ssttest "${PROJECT_SOURCE_DIR}/test/SstTest.cpp")
#link
target_link_libraries(awaittest sharpen)
target_link_libraries(timertest sharpen)
target_link_libraries(consoletest sharpen)
target_link_libraries(paralleltest sharpen)
target_link_libraries(templatetest sharpen)
target_link_libraries(filetest sharpen)
target_link_libraries(networktest sharpen)
target_link_libraries(optiontest sharpen)
target_link_libraries(arenatest sharpen)
target_link_libraries(dummytypetest sharpen)
target_link_libraries(microrpctest sharpen)
target_link_libraries(quorumtest sharpen)
target_link_libraries(checksumtest sharpen)
target_link_libraries(bloomfiltertest sharpen)
target_link_libraries(base64test sharpen)
target_link_libraries(memorytabletest sharpen)
target_link_libraries(varinttest sharpen)
target_link_libraries(buffercomparetest sharpen)
target_link_libraries(intcasttest sharpen)
target_link_libraries(ssttest sharpen)
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
add_test(NAME arena_test COMMAND "./arenatest${extname}")
add_test(NAME dummy_type_test COMMAND "./dummytypetest${extname}")
add_test(NAME microrpc_test COMMAND "./microrpctest${extname}")
add_test(NAME quorum_test COMMAND "./quorumtest${extname}")
add_test(NAME checksum_test COMMAND "./checksumtest${extname}")
add_test(NAME bloomfilter_test COMMAND "./bloomfiltertest${extname}")
add_test(NAME base64_test COMMAND "./bloomfiltertest${extname}")
add_test(NAME memorytable_test COMMAND "./memorytabletest${extname}")
add_test(NAME varint_test COMMAND "./varinttest${extname}")
add_test(NAME buffercompare_test COMMAND "./buffercomparetest${extname}")
add_test(NAME intcast_test COMMAND "./intcasttest${extname}")
add_test(NAME sst_test COMMAND "./ssttest${extname}")