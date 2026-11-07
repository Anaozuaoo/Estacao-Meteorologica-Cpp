#ifndef ESP32I2CDRIVER_H
#define ESP32I2CDRIVER_H

#include "II2CDriver.h"
#include "driver/i2c.h"
#include "esp_err.h"

/**
 * @brief ESP32 implementation of II2CDriver using ESP-IDF I2C driver.
 */
class ESP32I2CDriver : public II2CDriver {
public:
    ESP32I2CDriver(i2c_port_t port = I2C_NUM_0,
                   int sda_pin = 21,
                   int scl_pin = 22,
                   uint32_t clk_speed = 100000);
    ~ESP32I2CDriver() override;

    bool init() override;
    bool writeReg(uint8_t address, uint8_t reg, uint8_t data) override;
    uint8_t readReg(uint8_t address, uint8_t reg) override;
    uint16_t readReg16(uint8_t address, uint8_t reg) override;
    bool readBytes(uint8_t address, uint8_t reg, uint8_t* buffer, size_t length) override;

private:
    i2c_port_t port;
    int sda;
    int scl;
    uint32_t clkSpeed;
    bool isInitialized;
};

#endif // ESP32I2CDRIVER_H
