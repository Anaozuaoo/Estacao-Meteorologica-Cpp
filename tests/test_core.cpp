#include "SensorBase.h"
#include "II2CDriver.h"
#include "BME280Sensor.h"
#include "DataQueue.h"
#include <iostream>
#include <cassert>
#include <map>

// Mock I2C driver to test core without real hardware
class MockI2CDriver : public II2CDriver {
public:
    std::map<uint8_t, uint8_t> registers;

    MockI2CDriver() {
        // Set default Chip ID (0x60 for BME280)
        registers[0xD0] = 0x60;

        // Set calibration registers with representative dummy values
        registers[0x88] = 0x70; registers[0x89] = 0x6B; // dig_T1
        registers[0x8A] = 0x48; registers[0x8B] = 0x67; // dig_T2
        registers[0x8C] = 0x18; registers[0x8D] = 0xFC; // dig_T3

        registers[0x8E] = 0x64; registers[0x8F] = 0x8E; // dig_P1
        registers[0x90] = 0xD0; registers[0x91] = 0xD5; // dig_P2
        registers[0x92] = 0xD0; registers[0x93] = 0x0B; // dig_P3
        registers[0x94] = 0x27; registers[0x95] = 0x1E; // dig_P4
        registers[0x96] = 0xF6; registers[0x97] = 0xFF; // dig_P5
        registers[0x98] = 0xF9; registers[0x99] = 0xFF; // dig_P6
        registers[0x9A] = 0xAC; registers[0x9B] = 0x26; // dig_P7
        registers[0x9C] = 0x0A; registers[0x9D] = 0x0A; // dig_P8
        registers[0x9E] = 0xBD; registers[0x9F] = 0x10; // dig_P9

        registers[0xA1] = 0x4B;                         // dig_H1
        registers[0xE1] = 0x6E; registers[0xE2] = 0x01; // dig_H2
        registers[0xE3] = 0x00;                         // dig_H3
        registers[0xE4] = 0x14; registers[0xE5] = 0x00; // dig_H4 & dig_H5
        registers[0xE6] = 0x00;
        registers[0xE7] = 0x1E;                         // dig_H6

        // Sensor raw readout registers
        registers[0xF7] = 0x51; registers[0xF8] = 0x82; registers[0xF9] = 0x00; // Pressure
        registers[0xFA] = 0x83; registers[0xFB] = 0x40; registers[0xFC] = 0x00; // Temperature
        registers[0xFD] = 0x7E; registers[0xFE] = 0x90;                         // Humidity
    }

    bool init() override { return true; }

    bool writeReg(uint8_t address, uint8_t reg, uint8_t data) override {
        (void)address;
        registers[reg] = data;
        return true;
    }

    uint8_t readReg(uint8_t address, uint8_t reg) override {
        (void)address;
        if (registers.find(reg) != registers.end()) {
            return registers[reg];
        }
        return 0;
    }

    uint16_t readReg16(uint8_t address, uint8_t reg) override {
        uint8_t lsb = readReg(address, reg);
        uint8_t msb = readReg(address, reg + 1);
        return static_cast<uint16_t>((msb << 8) | lsb);
    }
};

int main() {
    std::cout << "=== Running Core Decoupling Unit Test ===" << std::endl;

    MockI2CDriver mockI2C;
    BME280Sensor bme("BME280_Test", mockI2C, 0x76);

    // 1. Test initialization
    bool initOk = bme.init();
    assert(initOk && "BME280 init failed!");
    std::cout << "[PASS] Sensor init with mock I2C" << std::endl;

    // 2. Test reading and calculations
    bool readOk = bme.readData();
    assert(readOk && "BME280 readData failed!");
    SensorData data = bme.getReading();
    std::cout << "[PASS] Reading acquired: T=" << data.temperature 
              << " C, P=" << data.pressure << " hPa, H=" << data.humidity << " %" << std::endl;

    // 3. Test polymorphic usage through SensorBase pointer
    SensorBase* basePtr = &bme;
    basePtr->printData();
    assert(basePtr->getId() == "BME280_Test");
    std::cout << "[PASS] Polymorphic SensorBase interface confirmed" << std::endl;

    // 4. Test DataQueue template thread safety & FIFO operations
    DataQueue<SensorData> queue(10);
    assert(queue.isEmpty());
    assert(queue.push(data));
    assert(!queue.isEmpty());
    assert(queue.size() == 1);

    SensorData poppedData;
    assert(queue.pop(poppedData));
    assert(poppedData.temperature == data.temperature);
    assert(queue.isEmpty());
    std::cout << "[PASS] DataQueue push/pop operations confirmed" << std::endl;

    std::cout << "All core unit tests passed successfully!" << std::endl;
    return 0;
}
