#include "STM32I2CDriver.h"
#include <cstdio>

#if !defined(STM32F4) && !defined(STM32F1) && !defined(STM32L4) && !defined(STM32G4)
// Fallback weak implementation for host building & testing
HAL_StatusTypeDef __attribute__((weak)) HAL_Init(void) { return HAL_OK; }
HAL_StatusTypeDef __attribute__((weak)) HAL_I2C_Mem_Write(I2C_HandleTypeDef*, uint16_t, uint16_t, uint16_t, uint8_t*, uint16_t, uint32_t) {
    return HAL_OK;
}
HAL_StatusTypeDef __attribute__((weak)) HAL_I2C_Mem_Read(I2C_HandleTypeDef*, uint16_t, uint16_t, uint16_t, uint8_t* pData, uint16_t Size, uint32_t) {
    for (uint16_t i = 0; i < Size; ++i) pData[i] = 0;
    return HAL_OK;
}
HAL_StatusTypeDef __attribute__((weak)) HAL_I2C_IsDeviceReady(I2C_HandleTypeDef*, uint16_t, uint16_t, uint32_t) {
    return HAL_OK;
}
HAL_StatusTypeDef __attribute__((weak)) HAL_UART_Transmit(UART_HandleTypeDef*, const uint8_t* pData, uint16_t Size, uint32_t) {
    std::fwrite(pData, 1, Size, stdout);
    return HAL_OK;
}
void __attribute__((weak)) HAL_Delay(uint32_t) {}
uint32_t __attribute__((weak)) HAL_GetTick(void) { return 0; }
#endif

STM32I2CDriver::STM32I2CDriver(I2C_HandleTypeDef* hi2cHandle, uint32_t timeoutMs)
    : hi2c(hi2cHandle), timeout(timeoutMs) {
}

bool STM32I2CDriver::init() {
    if (hi2c == nullptr) {
        return false;
    }
    return true;
}

bool STM32I2CDriver::writeReg(uint8_t address, uint8_t reg, uint8_t data) {
    if (hi2c == nullptr) return false;

    // STM32 HAL expects 7-bit I2C address left-shifted by 1
    uint16_t devAddr = static_cast<uint16_t>(address << 1);
    uint8_t val = data;

    HAL_StatusTypeDef status = HAL_I2C_Mem_Write(
        hi2c,
        devAddr,
        reg,
        I2C_MEMADD_SIZE_8BIT,
        &val,
        1,
        timeout
    );
    return (status == HAL_OK);
}

uint8_t STM32I2CDriver::readReg(uint8_t address, uint8_t reg) {
    if (hi2c == nullptr) return 0;

    uint16_t devAddr = static_cast<uint16_t>(address << 1);
    uint8_t val = 0;

    HAL_StatusTypeDef status = HAL_I2C_Mem_Read(
        hi2c,
        devAddr,
        reg,
        I2C_MEMADD_SIZE_8BIT,
        &val,
        1,
        timeout
    );

    if (status != HAL_OK) {
        return 0;
    }
    return val;
}

uint16_t STM32I2CDriver::readReg16(uint8_t address, uint8_t reg) {
    if (hi2c == nullptr) return 0;

    uint16_t devAddr = static_cast<uint16_t>(address << 1);
    uint8_t buf[2] = {0, 0};

    HAL_StatusTypeDef status = HAL_I2C_Mem_Read(
        hi2c,
        devAddr,
        reg,
        I2C_MEMADD_SIZE_8BIT,
        buf,
        2,
        timeout
    );

    if (status != HAL_OK) {
        return 0;
    }
    // Little-Endian (LSB at reg, MSB at reg+1)
    return static_cast<uint16_t>((buf[1] << 8) | buf[0]);
}

bool STM32I2CDriver::readBytes(uint8_t address, uint8_t reg, uint8_t* buffer, size_t length) {
    if (hi2c == nullptr || buffer == nullptr || length == 0) return false;

    uint16_t devAddr = static_cast<uint16_t>(address << 1);
    HAL_StatusTypeDef status = HAL_I2C_Mem_Read(
        hi2c,
        devAddr,
        reg,
        I2C_MEMADD_SIZE_8BIT,
        buffer,
        static_cast<uint16_t>(length),
        timeout
    );

    return (status == HAL_OK);
}
