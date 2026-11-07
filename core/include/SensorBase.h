#ifndef SENSORBASE_H
#define SENSORBASE_H

#include <string>

/**
 * @brief Global sensor data structure holding environmental measurements.
 */
struct SensorData {
    float temperature = 0.0f; // Temperature in Celsius (°C)
    float humidity    = 0.0f; // Relative humidity in percentage (%)
    float pressure    = 0.0f; // Atmospheric pressure in hectopascals (hPa)
};

/**
 * @brief Abstract base class establishing the contract for environmental sensors.
 *
 * Implements the open-closed principle: new sensors (e.g., BMP280, DHT22, SHT31)
 * can be added to the system by deriving from SensorBase without modifying the
 * controller or application layer.
 */
class SensorBase {
public:
    virtual ~SensorBase() = default;

    /**
     * @brief Initialize and detect the sensor hardware.
     * @return true on success, false on failure.
     */
    virtual bool init() = 0;

    /**
     * @brief Sample sensor registers, convert and compensate values.
     * @return true on success, false on communication failure.
     */
    virtual bool readData() = 0;

    /**
     * @brief Get the most recent valid reading.
     * @return SensorData structure with latest measurements.
     */
    virtual SensorData getReading() = 0;

    /**
     * @brief Output the latest reading (e.g., console log or formatted serial).
     */
    virtual void printData() = 0;

    /**
     * @brief Get sensor identifier string.
     */
    const std::string& getId() const { return id; }

protected:
    std::string id;
    SensorData lastReading;
};

#endif // SENSORBASE_H
