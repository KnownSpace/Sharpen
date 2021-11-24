cmake_minimum_required(VERSION 3.14.0)

include_directories("${PROJECT_SOURCE_DIR}/include")

add_executable(MicroEncodeBenchmark "${PROJECT_SOURCE_DIR}/examples/MicroRpc/EncodeBenchmark.cpp")
add_executable(MicroDecodeBenchmark "${PROJECT_SOURCE_DIR}/examples/MicroRpc/DecodeBenchmark.cpp")
add_executable(MicroClient "${PROJECT_SOURCE_DIR}/examples/MicroRpc/MicroRpcClient.cpp")
add_executable(MicroServer "${PROJECT_SOURCE_DIR}/examples/MicroRpc/MicroRpcServer.cpp")
add_executable(QuorumClient "${PROJECT_SOURCE_DIR}/examples/MicroRpc/QuorumClient.cpp")

target_link_libraries(MicroEncodeBenchmark sharpen)
target_link_libraries(MicroDecodeBenchmark sharpen)
target_link_libraries(MicroClient sharpen)
target_link_libraries(MicroServer sharpen)
target_link_libraries(QuorumClient sharpen)