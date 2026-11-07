cmake_minimum_required(VERSION 3.16)
project(stm32_weather_station CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Core sources & target sources
set(TARGET_SRCS
    src/main.cpp
    src/STM32I2CDriver.cpp
    src/STM32DeviceController.cpp
    ../../core/src/BME280Sensor.cpp
)

add_executable(stm32_weather_station ${TARGET_SRCS})

target_include_directories(stm32_weather_station PRIVATE
    include
    ../../core/include
)

# If compiling with ARM GCC toolchain, set MCU target flags:
if(CMAKE_C_COMPILER_ID MATCHES "GNU" AND CMAKE_SYSTEM_PROCESSOR MATCHES "arm")
    target_compile_options(stm32_weather_station PRIVATE
        -mcpu=cortex-m4
        -mthumb
        -mfpu=fpv4-sp-d16
        -mfloat-abi=hard
        -Wall
        -Wextra
    )
    target_link_options(stm32_weather_station PRIVATE
        -mcpu=cortex-m4
        -mthumb
        -mfpu=fpv4-sp-d16
        -mfloat-abi=hard
        -specs=nano.specs
        -specs=nosys.specs
    )
endif()
