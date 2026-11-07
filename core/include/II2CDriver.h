#ifndef II2CDRIVER_H
#define II2CDRIVER_H

#include <cstdint>
#include <cstddef>

/**
 * @brief Hardware Abstraction Layer (HAL) interface for I2C master operations.
 *
 * This pure virtual interface decouples core device drivers (such as BME280)
 * from platform-specific SDKs (ESP-IDF, STM32 HAL, Arduino, or Linux i2c-dev).
 */
class II2CDriver {
public:
    virtual ~II2CDriver() = default;

    /**
     * @brief Initialize the hardware I2C bus peripheral.
     * @return true on success, false on error.
     */
    virtual bool init() = 0;

    /**
     * @brief Write an 8-bit value to a device register.
     * @param address 7-bit I2C device address.
     * @param reg Target register address.
     * @param data Byte value to write.
     * @return true on success, false on communication error.
     */
    virtual bool writeReg(uint8_t address, uint8_t reg, uint8_t data) = 0;

    /**
     * @brief Read a single 8-bit value from a device register.
     * @param address 7-bit I2C device address.
     * @param reg Target register address.
     * @return Byte value read.
     */
    virtual uint8_t readReg(uint8_t address, uint8_t reg) = 0;

    /**
     * @brief Read a 16-bit word from register (Little-Endian: LSB first, MSB second).
     * @param address 7-bit I2C device address.
     * @param reg Target register start address.
     * @return 16-bit word read.
     */
    virtual uint16_t readReg16(uint8_t address, uint8_t reg) = 0;

    /**
     * @brief Read a sequence of bytes starting from a register.
     * @param address 7-bit I2C device address.
     * @param reg Starting register address.
     * @param buffer Destination buffer.
     * @param length Number of bytes to read.
     * @return true on success, false on error.
     */
    virtual bool readBytes(uint8_t address, uint8_t reg, uint8_t* buffer, size_t length) {
        if (!buffer || length == 0) return false;
        for (size_t i = 0; i < length; ++i) {
            buffer[i] = readReg(address, static_cast<uint8_t>(reg + i));
        }
        return true;
    }
};

#endif // II2CDRIVER_H
