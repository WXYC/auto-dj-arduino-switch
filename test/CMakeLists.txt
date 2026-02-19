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

# Build utils.cpp from the sketch directory, using our shim instead of real Arduino.h
set(SKETCH_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../auto-dj-arduino-switch)

add_library(utils STATIC ${SKETCH_DIR}/utils.cpp)
target_include_directories(utils PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/shim   # Arduino.h shim
    ${SKETCH_DIR}                       # utils.h
)

enable_testing()

# Test executables
add_executable(test_url_encode test_url_encode.cpp)
target_link_libraries(test_url_encode PRIVATE utils GTest::gtest_main)

add_executable(test_location_parsing test_location_parsing.cpp)
target_link_libraries(test_location_parsing PRIVATE utils GTest::gtest_main)

add_executable(test_current_hour_ms test_current_hour_ms.cpp)
target_link_libraries(test_current_hour_ms PRIVATE utils GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(test_url_encode)
gtest_discover_tests(test_location_parsing)
gtest_discover_tests(test_current_hour_ms)
