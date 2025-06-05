cmake_minimum_required(VERSION 3.20) # 3.20 is for C++23
project(AndroidMemoryEditor)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED on)
string(APPEND CMAKE_CXX_FLAGS
    " -Wall"
    " -Werror=return-type"
    " -Werror=unused-result"
)

add_library(ame STATIC
    src/ame_memory.cpp
    src/ame_process.cpp
)
target_include_directories(ame PUBLIC include)

add_subdirectory(test)
