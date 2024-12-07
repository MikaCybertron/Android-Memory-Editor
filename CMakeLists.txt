cmake_minimum_required(VERSION 3.15)
project(Android-Memory-Editor)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED on)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

include_directories(${PROJECT_SOURCE_DIR}/inc)

aux_source_directory(src SRC_DIR)

add_executable(ame ${SRC_DIR})
