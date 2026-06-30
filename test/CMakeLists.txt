cmake_minimum_required(VERSION 3.14)
cmake_policy(SET CMP0169 OLD)
project(auto-dj-arduino-switch-tests LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

# GoogleTest
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.14.0
)
FetchContent_MakeAvailable(googletest)

# ArduinoJson v7 (header-only)
FetchContent_Declare(
    arduinojson
    GIT_REPOSITORY https://github.com/bblanchon/ArduinoJson.git
    GIT_TAG v7.1.0
)
FetchContent_GetProperties(arduinojson)
if(NOT arduinojson_POPULATED)
    FetchContent_Populate(arduinojson)
endif()

# ========== Sketch source directory ==========

set(SKETCH_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../auto-dj-arduino-switch)

# ========== Shim ==========

# Arduino shim (Serial instance, millis(), Stream methods)
add_library(arduino_shim STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/shim/shim.cpp
)
target_include_directories(arduino_shim PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/shim
)

# ========== Sketch module libraries ==========

# Pure logic (state_machine) -- no I/O dependencies. The reporter firmware no
# longer needs utils (url-encode / radioShowID / hour math were flowsheet-only).
add_library(sketch_logic STATIC
    ${SKETCH_DIR}/state_machine.cpp
)
target_include_directories(sketch_logic PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/shim   # Arduino.h shim
    ${SKETCH_DIR}                       # state_machine.h, config.h
)

# Relay monitor module
add_library(relay_monitor STATIC
    ${SKETCH_DIR}/relay_monitor.cpp
)
target_include_directories(relay_monitor PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/shim
    ${SKETCH_DIR}
)
target_compile_definitions(relay_monitor PRIVATE DESKTOP_TEST)
target_link_libraries(relay_monitor PUBLIC arduino_shim)

# Button monitor module
add_library(button_monitor STATIC
    ${SKETCH_DIR}/button_monitor.cpp
)
target_include_directories(button_monitor PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/shim
    ${SKETCH_DIR}
)
target_compile_definitions(button_monitor PRIVATE DESKTOP_TEST)
target_link_libraries(button_monitor PUBLIC arduino_shim)

# Management-protocol module (pure JSON assembly/parse; no sockets)
add_library(mgmt_protocol STATIC
    ${SKETCH_DIR}/mgmt_protocol.cpp
)
target_include_directories(mgmt_protocol PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/shim
    ${SKETCH_DIR}
    ${arduinojson_SOURCE_DIR}/src
)
target_compile_definitions(mgmt_protocol PRIVATE
    DESKTOP_TEST
    ARDUINOJSON_ENABLE_ARDUINO_STRING=1
    ARDUINOJSON_ENABLE_ARDUINO_STREAM=1
)
target_link_libraries(mgmt_protocol PUBLIC arduino_shim)

# NOTE: mgmt_client.cpp is excluded -- it #include <ArduinoWebsockets.h>, which
# is not available in CI. All message assembly/parsing lives in mgmt_protocol
# (above) and is fully tested here; mgmt_client is thin socket plumbing only.

# ========== Tests ==========

enable_testing()

add_executable(test_state_machine test_state_machine.cpp)
target_link_libraries(test_state_machine PRIVATE sketch_logic GTest::gtest_main)

add_executable(test_relay_monitor test_relay_monitor.cpp)
target_link_libraries(test_relay_monitor PRIVATE relay_monitor arduino_shim GTest::gtest_main)

add_executable(test_button_monitor test_button_monitor.cpp)
target_link_libraries(test_button_monitor PRIVATE button_monitor arduino_shim GTest::gtest_main)

add_executable(test_mgmt_protocol test_mgmt_protocol.cpp)
target_link_libraries(test_mgmt_protocol PRIVATE mgmt_protocol sketch_logic arduino_shim GTest::gtest_main)
target_include_directories(test_mgmt_protocol PRIVATE ${arduinojson_SOURCE_DIR}/src)
target_compile_definitions(test_mgmt_protocol PRIVATE
    ARDUINOJSON_ENABLE_ARDUINO_STRING=1
    ARDUINOJSON_ENABLE_ARDUINO_STREAM=1
)

include(GoogleTest)
gtest_discover_tests(test_state_machine)
gtest_discover_tests(test_relay_monitor)
gtest_discover_tests(test_button_monitor)
gtest_discover_tests(test_mgmt_protocol)
