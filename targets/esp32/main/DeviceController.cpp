#include "DeviceController.h"
#include "BME280Sensor.h"
#include "esp_log.h"

static const char* TAG = "DeviceController";

// Default I2C configuration for ESP32
#define I2C_SDA_PIN 21
#define I2C_SCL_PIN 22
#define I2C_PORT    I2C_NUM_0
#define BME_ADDR    0x76

DeviceController::DeviceController()
    : i2c(I2C_PORT, I2C_SDA_PIN, I2C_SCL_PIN),
      spiffs(),
      network(spiffs),
      queue(20),
      sensorTaskHandle(nullptr) {
}

DeviceController::~DeviceController() {
    if (sensorTaskHandle) {
        vTaskDelete(sensorTaskHandle);
        sensorTaskHandle = nullptr;
    }
    for (auto* sensor : sensors) {
        delete sensor;
    }
    sensors.clear();
}

void DeviceController::start() {
    ESP_LOGI(TAG, "Starting DeviceController on ESP32...");

    // 1. Initialize I2C Bus Hardware
    if (!i2c.init()) {
        ESP_LOGE(TAG, "Failed to initialize ESP32 I2C bus!");
    }

    // 2. Instantiate BME280 sensor with FreeRTOS delay lambda
    auto delayFn = [](uint32_t ms) {
        vTaskDelay(pdMS_TO_TICKS(ms));
    };

    SensorBase* bme = new BME280Sensor("BME280_Main", i2c, BME_ADDR, delayFn);
    if (bme->init()) {
        sensors.push_back(bme);
        ESP_LOGI(TAG, "Sensor 'BME280_Main' registered successfully.");
    } else {
        ESP_LOGE(TAG, "CRITICAL: BME280 sensor not detected on I2C bus (0x%02X)!", BME_ADDR);
    }

    // 3. Connect Network & Start HTTP Web Dashboard
    network.connect();

    // 4. Launch FreeRTOS Task for Periodic Sampling
    xTaskCreate(sensorReadTask, "SensorReadTask", 4096, this, 5, &sensorTaskHandle);
    ESP_LOGI(TAG, "SensorReadTask spawned successfully.");
}

void DeviceController::sensorReadTask(void* pvParameters) {
    auto* self = static_cast<DeviceController*>(pvParameters);

    while (true) {
        // Iterate polymorphically through all registered sensors
        for (SensorBase* sensor : self->sensors) {
            if (sensor->readData()) {
                SensorData data = sensor->getReading();

                // Store in thread-safe queue and push to network manager
                self->queue.push(data);
                self->network.updateData(data);

                sensor->printData();
            } else {
                ESP_LOGW(TAG, "Failed to read sensor '%s'", sensor->getId().c_str());
            }
        }

        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}
