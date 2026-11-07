#ifndef STM32I2CDRIVER_H
#define STM32I2CDRIVER_H

#include "II2CDriver.h"
#include "stm32_hal_compat.h"

/**
 * @brief STM32 implementation of II2CDriver using STM32 HAL I2C API.
 *
 * Implements writeReg, readReg, readReg16, and readBytes via:
 * - HAL_I2C_Mem_Write()
 * - HAL_I2C_Mem_Read()
 * - HAL_I2C_IsDeviceReady()
 */
class STM32I2CDriver : public II2CDriver {
public:
    explicit STM32I2CDriver(I2C_HandleTypeDef* hi2cHandle, uint32_t timeoutMs = 100);
    ~STM32I2CDriver() override = default;

    bool init() override;
    bool writeReg(uint8_t address, uint8_t reg, uint8_t data) override;
    uint8_t readReg(uint8_t address, uint8_t reg) override;
    uint16_t readReg16(uint8_t address, uint8_t reg) override;
    bool readBytes(uint8_t address, uint8_t reg, uint8_t* buffer, size_t length) override;

    void setHandle(I2C_HandleTypeDef* handle) { hi2c = handle; }

private:
    I2C_HandleTypeDef* hi2c;
    uint32_t timeout;
};

#endif // STM32I2CDRIVER_H
