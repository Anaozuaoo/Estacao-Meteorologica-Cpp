#ifndef STM32DEVICECONTROLLER_H
#define STM32DEVICECONTROLLER_H

#include "SensorBase.h"
#include "DataQueue.h"
#include "STM32I2CDriver.h"
#include "stm32_hal_compat.h"
#include <vector>

/**
 * @brief System Controller for STM32 targets.
 *
 * Coordinates:
 * - Hardware I2C Driver via STM32 HAL
 * - Polymorphic sensors (std::vector<SensorBase*>)
 * - Thread-safe DataQueue<SensorData>
 * - Telemetry output via UART
 */
class STM32DeviceController {
public:
    STM32DeviceController(I2C_HandleTypeDef* hi2c, UART_HandleTypeDef* huart = nullptr);
    ~STM32DeviceController();

    /**
     * @brief Initialize drivers, instantiate BME280 sensor, register into sensor list.
     */
    bool start();

    /**
     * @brief Periodic processing step (invoked from main loop or FreeRTOS task).
     */
    void process();

private:
    STM32I2CDriver i2c;
    UART_HandleTypeDef* huart;
    DataQueue<SensorData> queue;
    std::vector<SensorBase*> sensors;

    void sendUartLog(const char* message);
};

#endif // STM32DEVICECONTROLLER_H
