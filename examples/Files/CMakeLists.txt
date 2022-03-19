cmake_minimum_required(VERSION 3.14.0)

add_executable(Encoder "${PROJECT_SOURCE_DIR}/examples/Files/Encoder.cpp")

add_executable(Memcache "${PROJECT_SOURCE_DIR}/examples/Files/Memcache.cpp")

add_executable(SimpleKvDb "${PROJECT_SOURCE_DIR}/examples/Files/SimpleKvDb.cpp")

add_executable(SimpleLsmKvDb "${PROJECT_SOURCE_DIR}/examples/Files/SimpleLsmKvDb.cpp")

target_link_libraries(Encoder sharpen)
target_link_libraries(Memcache sharpen)
target_link_libraries(SimpleKvDb sharpen)
target_link_libraries(SimpleLsmKvDb sharpen)