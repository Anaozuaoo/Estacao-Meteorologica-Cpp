#include "ESP32I2CDriver.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char* TAG = "ESP32I2CDriver";

ESP32I2CDriver::ESP32I2CDriver(i2c_port_t port, int sda_pin, int scl_pin, uint32_t clk_speed)
    : port(port), sda(sda_pin), scl(scl_pin), clkSpeed(clk_speed), isInitialized(false) {
}

ESP32I2CDriver::~ESP32I2CDriver() {
    if (isInitialized) {
        i2c_driver_delete(port);
        isInitialized = false;
    }
}

bool ESP32I2CDriver::init() {
    if (isInitialized) {
        return true;
    }

    i2c_config_t conf = {};
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = sda;
    conf.scl_io_num = scl;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = clkSpeed;

    esp_err_t err = i2c_param_config(port, &conf);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "i2c_param_config failed: %s", esp_err_to_name(err));
        return false;
    }

    err = i2c_driver_install(port, conf.mode, 0, 0, 0);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "i2c_driver_install failed: %s", esp_err_to_name(err));
        return false;
    }

    isInitialized = true;
    ESP_LOGI(TAG, "I2C initialized on port %d (SDA: GPIO %d, SCL: GPIO %d, Speed: %lu Hz)",
             port, sda, scl, (unsigned long)clkSpeed);
    return true;
}

bool ESP32I2CDriver::writeReg(uint8_t address, uint8_t reg, uint8_t data) {
    if (!isInitialized && !init()) {
        return false;
    }

    uint8_t write_buf[2] = {reg, data};
    esp_err_t err = i2c_master_write_to_device(port, address, write_buf, sizeof(write_buf), pdMS_TO_TICKS(1000));
    return (err == ESP_OK);
}

uint8_t ESP32I2CDriver::readReg(uint8_t address, uint8_t reg) {
    if (!isInitialized && !init()) {
        return 0;
    }

    uint8_t data = 0;
    esp_err_t err = i2c_master_write_read_device(port, address, &reg, 1, &data, 1, pdMS_TO_TICKS(1000));
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "readReg failed at addr 0x%02X, reg 0x%02X (%s)", address, reg, esp_err_to_name(err));
        return 0;
    }
    return data;
}

uint16_t ESP32I2CDriver::readReg16(uint8_t address, uint8_t reg) {
    if (!isInitialized && !init()) {
        return 0;
    }

    uint8_t data[2] = {0, 0};
    esp_err_t err = i2c_master_write_read_device(port, address, &reg, 1, data, 2, pdMS_TO_TICKS(1000));
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "readReg16 failed at addr 0x%02X, reg 0x%02X (%s)", address, reg, esp_err_to_name(err));
        return 0;
    }
    // Little-Endian format (LSB at reg, MSB at reg+1)
    return static_cast<uint16_t>((data[1] << 8) | data[0]);
}

bool ESP32I2CDriver::readBytes(uint8_t address, uint8_t reg, uint8_t* buffer, size_t length) {
    if (!buffer || length == 0) return false;
    if (!isInitialized && !init()) return false;

    esp_err_t err = i2c_master_write_read_device(port, address, &reg, 1, buffer, length, pdMS_TO_TICKS(1000));
    return (err == ESP_OK);
}
