#ifndef DEVICECONTROLLER_H
#define DEVICECONTROLLER_H

#include "SensorBase.h"
#include "DataQueue.h"
#include "ESP32I2CDriver.h"
#include "SpiffsManager.h"
#include "NetworkManager.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <vector>

/**
 * @brief Main coordinator on ESP32 target.
 *
 * Demonstrates:
 * - Polymorphism via std::vector<SensorBase*>
 * - Dependency Injection of II2CDriver implementation into SensorBase
 * - Thread-safety via DataQueue<SensorData>
 * - Multi-task scheduling via FreeRTOS
 */
class DeviceController {
public:
    DeviceController();
    ~DeviceController();

    /**
     * @brief Initialize peripherals, mount storage, connect network and start tasks.
     */
    void start();

private:
    ESP32I2CDriver i2c;
    SpiffsManager spiffs;
    NetworkManager network;
    DataQueue<SensorData> queue;

    std::vector<SensorBase*> sensors;

    TaskHandle_t sensorTaskHandle;

    static void sensorReadTask(void* pvParameters);
};

#endif // DEVICECONTROLLER_H
