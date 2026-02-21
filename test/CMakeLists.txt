cmake_minimum_required(VERSION 3.14)
project(auto-dj-arduino-switch-tests LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# GoogleTest
include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.14.0
)
FetchContent_MakeAvailable(googletest)

# Build sketch pure-logic sources, using our shim instead of real Arduino.h
set(SKETCH_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../auto-dj-arduino-switch)

add_library(sketch_logic STATIC
    ${SKETCH_DIR}/utils.cpp
    ${SKETCH_DIR}/state_machine.cpp
)
target_include_directories(sketch_logic PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/shim   # Arduino.h shim
    ${SKETCH_DIR}                       # utils.h, state_machine.h
)

enable_testing()

# Test executables
add_executable(test_url_encode test_url_encode.cpp)
target_link_libraries(test_url_encode PRIVATE sketch_logic GTest::gtest_main)

add_executable(test_location_parsing test_location_parsing.cpp)
target_link_libraries(test_location_parsing PRIVATE sketch_logic GTest::gtest_main)

add_executable(test_current_hour_ms test_current_hour_ms.cpp)
target_link_libraries(test_current_hour_ms PRIVATE sketch_logic GTest::gtest_main)

add_executable(test_state_machine test_state_machine.cpp)
target_link_libraries(test_state_machine PRIVATE sketch_logic GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(test_url_encode)
gtest_discover_tests(test_location_parsing)
gtest_discover_tests(test_current_hour_ms)
gtest_discover_tests(test_state_machine)
