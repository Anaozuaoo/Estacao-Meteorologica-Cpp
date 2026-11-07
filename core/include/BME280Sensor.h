#ifndef BME280SENSOR_H
#define BME280SENSOR_H

#include "SensorBase.h"
#include "II2CDriver.h"
#include <string>
#include <cstdint>
#include <functional>

/**
 * @brief Concrete driver for Bosch BME280 Temperature, Humidity, and Pressure Sensor.
 *
 * Implements SensorBase. Relies purely on the II2CDriver abstraction, ensuring
 * portability across any embedded architecture (ESP32, STM32, etc.) or host simulator.
 */
class BME280Sensor : public SensorBase {
public:
    using DelayFunc = std::function<void(uint32_t ms)>;

    /**
     * @brief Construct a new BME280Sensor driver.
     * @param id Identifier for the sensor instance.
     * @param i2cDriver Reference to an II2CDriver implementation.
     * @param address 7-bit I2C device address (default 0x76, alternate 0x77).
     * @param delayFn Optional platform delay callback (e.g. FreeRTOS vTaskDelay or STM32 HAL_Delay).
     */
    BME280Sensor(std::string id,
                 II2CDriver& i2cDriver,
                 uint8_t address = 0x76,
                 DelayFunc delayFn = nullptr);

    bool init() override;
    bool readData() override;
    SensorData getReading() override;
    void printData() override;

    uint8_t getAddress() const { return address; }

private:
    II2CDriver& i2c;
    uint8_t address;
    DelayFunc delayMs;

    // Intermediate variable required by Bosch compensation formulas
    int32_t t_fine;

    // Factory calibration registers
    struct CalibrationData {
        // Temperature
        uint16_t dig_T1;
        int16_t  dig_T2;
        int16_t  dig_T3;

        // Pressure
        uint16_t dig_P1;
        int16_t  dig_P2;
        int16_t  dig_P3;
        int16_t  dig_P4;
        int16_t  dig_P5;
        int16_t  dig_P6;
        int16_t  dig_P7;
        int16_t  dig_P8;
        int16_t  dig_P9;

        // Humidity
        uint8_t  dig_H1;
        int16_t  dig_H2;
        uint8_t  dig_H3;
        int16_t  dig_H4;
        int16_t  dig_H5;
        int8_t   dig_H6;
    } calib;

    void readCalibrationData();
    void delay(uint32_t ms);

    // Official 64-bit Bosch compensation calculations
    float compensateTemperature(int32_t adc_T);
    float compensatePressure(int32_t adc_P);
    float compensateHumidity(int32_t adc_H);
};

#endif // BME280SENSOR_H
