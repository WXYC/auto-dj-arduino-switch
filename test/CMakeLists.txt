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

# ArduinoJson v7 (header-only; no CMakeLists.txt integration needed)
FetchContent_Declare(
    arduinojson
    GIT_REPOSITORY https://github.com/bblanchon/ArduinoJson.git
    GIT_TAG v7.1.0
)
FetchContent_GetProperties(arduinojson)
if(NOT arduinojson_POPULATED)
    FetchContent_Populate(arduinojson)
endif()

# ArduinoHttpClient (no CMakeLists.txt -- Arduino library)
FetchContent_Declare(
    arduinohttpclient
    GIT_REPOSITORY https://github.com/arduino-libraries/ArduinoHttpClient.git
    GIT_TAG 0.6.1
)
FetchContent_GetProperties(arduinohttpclient)
if(NOT arduinohttpclient_POPULATED)
    FetchContent_Populate(arduinohttpclient)
endif()

# ========== Sketch source directory ==========

set(SKETCH_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../auto-dj-arduino-switch)

# ========== Shim & third-party libraries ==========

# Arduino shim (Serial instance, millis(), Stream methods)
add_library(arduino_shim STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/shim/shim.cpp
)
target_include_directories(arduino_shim PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/shim
)

# ArduinoHttpClient compiled against our shim
add_library(httpclient STATIC
    ${arduinohttpclient_SOURCE_DIR}/src/HttpClient.cpp
    ${arduinohttpclient_SOURCE_DIR}/src/b64.cpp
)
target_include_directories(httpclient PUBLIC
    ${arduinohttpclient_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/shim
)
target_link_libraries(httpclient PUBLIC arduino_shim)

# ========== Sketch module libraries ==========

# Pure logic (utils, state_machine) -- no I/O dependencies
add_library(sketch_logic STATIC
    ${SKETCH_DIR}/utils.cpp
    ${SKETCH_DIR}/state_machine.cpp
)
target_include_directories(sketch_logic PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/shim   # Arduino.h shim
    ${SKETCH_DIR}                       # utils.h, state_machine.h, config.h
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

# AzuraCast client module
add_library(azuracast_client STATIC
    ${SKETCH_DIR}/azuracast_client.cpp
)
target_include_directories(azuracast_client PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/shim
    ${SKETCH_DIR}
    ${arduinojson_SOURCE_DIR}/src
)
target_compile_definitions(azuracast_client PRIVATE
    DESKTOP_TEST
    ARDUINOJSON_ENABLE_ARDUINO_STRING=1
    ARDUINOJSON_ENABLE_ARDUINO_STREAM=1
)
target_link_libraries(azuracast_client PUBLIC arduino_shim httpclient)

# Flowsheet client module
add_library(flowsheet_client STATIC
    ${SKETCH_DIR}/flowsheet_client.cpp
)
target_include_directories(flowsheet_client PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/shim
    ${SKETCH_DIR}
)
target_compile_definitions(flowsheet_client PRIVATE DESKTOP_TEST)
target_link_libraries(flowsheet_client PUBLIC arduino_shim httpclient sketch_logic)

# ========== Tests ==========

enable_testing()

# Existing tests
add_executable(test_url_encode test_url_encode.cpp)
target_link_libraries(test_url_encode PRIVATE sketch_logic GTest::gtest_main)

add_executable(test_location_parsing test_location_parsing.cpp)
target_link_libraries(test_location_parsing PRIVATE sketch_logic GTest::gtest_main)

add_executable(test_current_hour_ms test_current_hour_ms.cpp)
target_link_libraries(test_current_hour_ms PRIVATE sketch_logic GTest::gtest_main)

add_executable(test_state_machine test_state_machine.cpp)
target_link_libraries(test_state_machine PRIVATE sketch_logic GTest::gtest_main)

# Module tests
add_executable(test_relay_monitor test_relay_monitor.cpp)
target_link_libraries(test_relay_monitor PRIVATE relay_monitor arduino_shim GTest::gtest_main)

add_executable(test_azuracast_client test_azuracast_client.cpp)
target_link_libraries(test_azuracast_client PRIVATE azuracast_client arduino_shim GTest::gtest_main)
target_include_directories(test_azuracast_client PRIVATE ${arduinojson_SOURCE_DIR}/src)
target_compile_definitions(test_azuracast_client PRIVATE
    ARDUINOJSON_ENABLE_ARDUINO_STRING=1
    ARDUINOJSON_ENABLE_ARDUINO_STREAM=1
)

add_executable(test_flowsheet_client test_flowsheet_client.cpp)
target_link_libraries(test_flowsheet_client PRIVATE flowsheet_client arduino_shim GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(test_url_encode)
gtest_discover_tests(test_location_parsing)
gtest_discover_tests(test_current_hour_ms)
gtest_discover_tests(test_state_machine)
gtest_discover_tests(test_relay_monitor)
gtest_discover_tests(test_azuracast_client)
gtest_discover_tests(test_flowsheet_client)
