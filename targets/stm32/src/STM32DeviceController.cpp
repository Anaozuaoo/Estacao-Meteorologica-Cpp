#include "STM32DeviceController.h"
#include "BME280Sensor.h"
#include <cstdio>
#include <cstring>

#define STM32_BME280_ADDR 0x76

STM32DeviceController::STM32DeviceController(I2C_HandleTypeDef* hi2cHandle, UART_HandleTypeDef* huartHandle)
    : i2c(hi2cHandle), huart(huartHandle), queue(20) {
}

STM32DeviceController::~STM32DeviceController() {
    for (auto* s : sensors) {
        delete s;
    }
    sensors.clear();
}

bool STM32DeviceController::start() {
    sendUartLog("\r\n==============================================\r\n");
    sendUartLog("[STM32] Weather Station Controller Starting...\r\n");
    sendUartLog("==============================================\r\n");

    if (!i2c.init()) {
        sendUartLog("[STM32] ERROR: I2C initialization failed.\r\n");
        return false;
    }

    // Pass HAL_Delay via lambda into BME280 driver
    auto delayFn = [](uint32_t ms) {
        HAL_Delay(ms);
    };

    SensorBase* bme = new BME280Sensor("BME280_STM32", i2c, STM32_BME280_ADDR, delayFn);
    if (bme->init()) {
        sensors.push_back(bme);
        sendUartLog("[STM32] Sensor BME280 registered successfully.\r\n");
        return true;
    } else {
        sendUartLog("[STM32] ERROR: Sensor BME280 not found on I2C bus!\r\n");
        return false;
    }
}

void STM32DeviceController::process() {
    for (SensorBase* sensor : sensors) {
        if (sensor->readData()) {
            SensorData data = sensor->getReading();
            queue.push(data);

            char msg[128];
            std::snprintf(msg, sizeof(msg),
                          "[STM32 Telemetry] ID: %s | Temp: %.2f C | Pres: %.2f hPa | Hum: %.2f %%\r\n",
                          sensor->getId().c_str(),
                          data.temperature,
                          data.pressure,
                          data.humidity);
            sendUartLog(msg);
        } else {
            sendUartLog("[STM32] WARNING: Failed to read from sensor!\r\n");
        }
    }
}

void STM32DeviceController::sendUartLog(const char* message) {
    if (!message) return;
    if (huart != nullptr) {
        HAL_UART_Transmit(huart, reinterpret_cast<const uint8_t*>(message), static_cast<uint16_t>(std::strlen(message)), 100);
    } else {
        std::printf("%s", message);
    }
}
