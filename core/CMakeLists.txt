cmake_minimum_required(VERSION 3.16)
project(weather_station_core CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(weather_station_core STATIC
    src/BME280Sensor.cpp
)

target_include_directories(weather_station_core PUBLIC
    include
)
