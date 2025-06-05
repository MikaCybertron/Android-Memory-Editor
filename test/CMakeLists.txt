set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

add_executable(ame_test main.cpp)
target_link_libraries(ame_test PRIVATE ame)
