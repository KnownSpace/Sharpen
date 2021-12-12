cmake_minimum_required(VERSION 3.14.0)

add_executable(RestfulClient "${PROJECT_SOURCE_DIR}/examples/Restful/RestfulClient.cpp")

add_executable(RestfulServer "${PROJECT_SOURCE_DIR}/examples/Restful/RestfulServer.cpp")

target_link_libraries(RestfulClient sharpen)

target_link_libraries(RestfulServer sharpen)