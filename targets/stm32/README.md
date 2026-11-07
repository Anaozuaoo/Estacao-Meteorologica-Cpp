# STM32 Target - IoT Weather Station

This target provides the embedded firmware implementation for STMicroelectronics STM32 microcontrollers (e.g., STM32F4, STM32F1, STM32L4, STM32G4 families).

## Hardware & Wiring

### BME280 Sensor Wiring (I2C1)
| BME280 Pin | STM32 Nucleo-64 (e.g. NUCLEO-F401RE / F411RE) | STM32 Blue Pill (STM32F103C8T6) |
|------------|----------------------------------------------|---------------------------------|
| **VCC**    | 3.3V                                         | 3.3V                            |
| **GND**    | GND                                          | GND                             |
| **SCL**    | PB8 (or PB6)                                 | PB6                             |
| **SDA**    | PB9 (or PB7)                                 | PB7                             |
| **CSB**    | 3.3V (selects I2C mode)                      | 3.3V                            |
| **SDO**    | GND (address 0x76) or 3.3V (address 0x77)     | GND (0x76) / 3.3V (0x77)        |

> **Note:** If your breakout board does not include pull-up resistors on SDA/SCL, enable internal pull-ups in GPIO configuration or add external 4.7kΩ pull-up resistors to 3.3V.

### Telemetry Output (USART2)
- **TX**: PA2 (connected to ST-LINK Virtual COM Port on Nucleo boards)
- **RX**: PA3
- **Baud Rate**: 115200 bps, 8N1

## Architecture & Integration
- Uses `STM32I2CDriver` implementing `II2CDriver` via STM32 HAL (`HAL_I2C_Mem_Write`, `HAL_I2C_Mem_Read`).
- Uses `core/BME280Sensor` polymorphically through `core/SensorBase`.
- Passes platform delay `HAL_Delay` to the sensor driver via lambda closure.
- Protects queued sensor readings using `DataQueue<SensorData>`.

## Building with CMake / ARM GCC
```bash
mkdir build && cd build
cmake -DCMAKE_TOOLCHAIN_FILE=../cmake/arm-none-eabi.cmake ..
make -j4
```

## Flashing with OpenOCD or STM32CubeProgrammer
```bash
openocd -f interface/stlink.cfg -f target/stm32f4x.cfg -c "program stm32_weather_station.elf verify reset exit"
```
Or use the ST-LINK CLI:
```bash
STM32_Programmer_CLI -c port=SWD -w stm32_weather_station.bin 0x08000000 -v -rst
```
