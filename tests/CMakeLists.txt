cmake_minimum_required(VERSION 3.16)
project(core_tests CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(test_core
    test_core.cpp
    ../core/src/BME280Sensor.cpp
)

target_include_directories(test_core PRIVATE
    ../core/include
)

enable_testing()
add_test(NAME CoreUnitTest COMMAND test_core)
