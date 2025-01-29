cmake_minimum_required(VERSION 3.12) # 3.12 is for C++20
project(AndroidMemoryEditor)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED on)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

aux_source_directory(src SRC_DIR)
add_executable(ame ${SRC_DIR})
target_include_directories(ame PRIVATE inc)
