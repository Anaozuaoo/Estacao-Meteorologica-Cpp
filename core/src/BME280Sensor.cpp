#include "BME280Sensor.h"
#include <cstdio>

BME280Sensor::BME280Sensor(std::string id,
                           II2CDriver& i2cDriver,
                           uint8_t address,
                           DelayFunc delayFn)
    : i2c(i2cDriver), address(address), delayMs(delayFn), t_fine(0) {
    this->id = id;
}

void BME280Sensor::delay(uint32_t ms) {
    if (delayMs) {
        delayMs(ms);
    } else {
        // Fallback simple cycle spin if no RTOS delay callback was injected
        volatile uint32_t count = ms * 8000;
        while (count--) {
            #if defined(__GNUC__)
            __asm__ __volatile__("");
            #endif
        }
    }
}

bool BME280Sensor::init() {
    // 1. Read Chip ID (0xD0)
    uint8_t chipId = i2c.readReg(address, 0xD0);
    // 0x60 = BME280, 0x58 = BMP280
    if (chipId != 0x60 && chipId != 0x58) {
        std::printf("[BME280] ERROR: Device not found at 0x%02X (read ID: 0x%02X, expected 0x60 or 0x58)\n", address, chipId);
        return false;
    }

    // 2. Soft Reset (0xE0 -> 0xB6)
    i2c.writeReg(address, 0xE0, 0xB6);
    delay(100);

    // 3. Read Factory Calibration Data
    readCalibrationData();

    // 4. Configure Sampling and Power Mode
    // ctrl_hum (0xF2) = 0x01 (Humidity oversampling x1)
    i2c.writeReg(address, 0xF2, 0x01);
    // ctrl_meas (0xF4) = 0x27 (Pressure x1, Temp x1, Normal Mode)
    i2c.writeReg(address, 0xF4, 0x27);
    // config (0xF5) = 0xA0 (Standby 1000ms, Filter OFF)
    i2c.writeReg(address, 0xF5, 0xA0);

    std::printf("[BME280] Sensor '%s' initialized at 0x%02X (Chip ID: 0x%02X)\n", id.c_str(), address, chipId);
    return true;
}

void BME280Sensor::readCalibrationData() {
    // Temperature & Pressure calibration (0x88 to 0x9E)
    calib.dig_T1 = i2c.readReg16(address, 0x88);
    calib.dig_T2 = static_cast<int16_t>(i2c.readReg16(address, 0x8A));
    calib.dig_T3 = static_cast<int16_t>(i2c.readReg16(address, 0x8C));

    calib.dig_P1 = i2c.readReg16(address, 0x8E);
    calib.dig_P2 = static_cast<int16_t>(i2c.readReg16(address, 0x90));
    calib.dig_P3 = static_cast<int16_t>(i2c.readReg16(address, 0x92));
    calib.dig_P4 = static_cast<int16_t>(i2c.readReg16(address, 0x94));
    calib.dig_P5 = static_cast<int16_t>(i2c.readReg16(address, 0x96));
    calib.dig_P6 = static_cast<int16_t>(i2c.readReg16(address, 0x98));
    calib.dig_P7 = static_cast<int16_t>(i2c.readReg16(address, 0x9A));
    calib.dig_P8 = static_cast<int16_t>(i2c.readReg16(address, 0x9C));
    calib.dig_P9 = static_cast<int16_t>(i2c.readReg16(address, 0x9E));

    // Humidity calibration (0xA1, 0xE1 - 0xE7)
    calib.dig_H1 = i2c.readReg(address, 0xA1);
    calib.dig_H2 = static_cast<int16_t>(i2c.readReg16(address, 0xE1));
    calib.dig_H3 = i2c.readReg(address, 0xE3);

    uint8_t e4 = i2c.readReg(address, 0xE4);
    uint8_t e5 = i2c.readReg(address, 0xE5);
    uint8_t e6 = i2c.readReg(address, 0xE6);

    calib.dig_H4 = static_cast<int16_t>((static_cast<uint16_t>(e4) << 4) | (e5 & 0x0F));
    calib.dig_H5 = static_cast<int16_t>((static_cast<uint16_t>(e6) << 4) | (e5 >> 4));
    calib.dig_H6 = static_cast<int8_t>(i2c.readReg(address, 0xE7));
}

bool BME280Sensor::readData() {
    uint8_t p_msb  = i2c.readReg(address, 0xF7);
    uint8_t p_lsb  = i2c.readReg(address, 0xF8);
    uint8_t p_xlsb = i2c.readReg(address, 0xF9);

    uint8_t t_msb  = i2c.readReg(address, 0xFA);
    uint8_t t_lsb  = i2c.readReg(address, 0xFB);
    uint8_t t_xlsb = i2c.readReg(address, 0xFC);

    uint8_t h_msb  = i2c.readReg(address, 0xFD);
    uint8_t h_lsb  = i2c.readReg(address, 0xFE);

    int32_t adc_P = (static_cast<int32_t>(p_msb) << 12) |
                    (static_cast<int32_t>(p_lsb) << 4)  |
                    (static_cast<int32_t>(p_xlsb) >> 4);

    int32_t adc_T = (static_cast<int32_t>(t_msb) << 12) |
                    (static_cast<int32_t>(t_lsb) << 4)  |
                    (static_cast<int32_t>(t_xlsb) >> 4);

    int32_t adc_H = (static_cast<int32_t>(h_msb) << 8)  |
                    static_cast<int32_t>(h_lsb);

    // Temperature must be calculated first to populate t_fine for pressure & humidity
    lastReading.temperature = compensateTemperature(adc_T);
    lastReading.pressure    = compensatePressure(adc_P);
    lastReading.humidity    = compensateHumidity(adc_H);

    return true;
}

SensorData BME280Sensor::getReading() {
    return lastReading;
}

void BME280Sensor::printData() {
    std::printf("[%s] T: %.2f °C | P: %.2f hPa | H: %.2f %%\n",
                id.c_str(),
                lastReading.temperature,
                lastReading.pressure,
                lastReading.humidity);
}

// --- Official Bosch Compensation Formulas ---

float BME280Sensor::compensateTemperature(int32_t adc_T) {
    int32_t var1, var2;
    var1 = ((((adc_T >> 3) - (static_cast<int32_t>(calib.dig_T1) << 1))) *
            (static_cast<int32_t>(calib.dig_T2))) >> 11;
    var2 = (((((adc_T >> 4) - static_cast<int32_t>(calib.dig_T1)) *
              ((adc_T >> 4) - static_cast<int32_t>(calib.dig_T1))) >> 12) *
            static_cast<int32_t>(calib.dig_T3)) >> 14;
    t_fine = var1 + var2;

    return static_cast<float>((t_fine * 5 + 128) >> 8) / 100.0f;
}

float BME280Sensor::compensatePressure(int32_t adc_P) {
    int64_t var1, var2, p;
    var1 = static_cast<int64_t>(t_fine) - 128000;
    var2 = var1 * var1 * static_cast<int64_t>(calib.dig_P6);
    var2 = var2 + ((var1 * static_cast<int64_t>(calib.dig_P5)) << 17);
    var2 = var2 + (static_cast<int64_t>(calib.dig_P4) << 35);
    var1 = ((var1 * var1 * static_cast<int64_t>(calib.dig_P3)) >> 8) +
           ((var1 * static_cast<int64_t>(calib.dig_P2)) << 12);
    var1 = (((static_cast<int64_t>(1) << 47) + var1) * static_cast<int64_t>(calib.dig_P1)) >> 33;

    if (var1 == 0) return 0.0f; // Prevent division by zero

    p = 1048576 - adc_P;
    p = (((p << 31) - var2) * 3125) / var1;
    var1 = (static_cast<int64_t>(calib.dig_P9) * (p >> 13) * (p >> 13)) >> 25;
    var2 = (static_cast<int64_t>(calib.dig_P8) * p) >> 19;
    p = ((p + var1 + var2) >> 8) + (static_cast<int64_t>(calib.dig_P7) << 4);

    return static_cast<float>(p) / 25600.0f; // Return in hPa
}

float BME280Sensor::compensateHumidity(int32_t adc_H) {
    int32_t v_x1_u32r;
    v_x1_u32r = (t_fine - static_cast<int32_t>(76800));
    v_x1_u32r = (((((adc_H << 14) - (static_cast<int32_t>(calib.dig_H4) << 20) -
                    (static_cast<int32_t>(calib.dig_H5) * v_x1_u32r)) +
                   static_cast<int32_t>(16384)) >> 15) *
                 (((((((v_x1_u32r * static_cast<int32_t>(calib.dig_H6)) >> 10) *
                      (((v_x1_u32r * static_cast<int32_t>(calib.dig_H3)) >> 11) + static_cast<int32_t>(32768))) >> 10) +
                    static_cast<int32_t>(2097152)) *
                   static_cast<int32_t>(calib.dig_H2) + 8192) >> 14));
    v_x1_u32r = (v_x1_u32r - (((((v_x1_u32r >> 15) * (v_x1_u32r >> 15)) >> 7) *
                               static_cast<int32_t>(calib.dig_H1)) >> 4));
    v_x1_u32r = (v_x1_u32r < 0 ? 0 : v_x1_u32r);
    v_x1_u32r = (v_x1_u32r > 419430400 ? 419430400 : v_x1_u32r);

    return static_cast<float>(v_x1_u32r >> 12) / 1024.0f;
}
