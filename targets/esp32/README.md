# ESP32 Target - IoT Weather Station

This target provides the embedded firmware implementation for Espressif ESP32 microcontrollers using ESP-IDF.

## Features
- **Sensor Driver:** BME280 temperature, humidity, and atmospheric pressure reading over hardware I2C (`ESP32I2CDriver` implementing `II2CDriver`).
- **Real-Time Tasking:** FreeRTOS task `SensorReadTask` sampling every 2 seconds with non-blocking delays (`vTaskDelay`).
- **Data Safety:** Thread-safe communication buffer using `DataQueue<SensorData>` with FreeRTOS mutex.
- **Web Dashboard:** Native embedded HTTP Web Server (`esp_http_server`) serving dynamic JSON at `/api/dados` and HTML5 dashboard at `/`.
- **SPIFFS Storage:** Static assets (`site_data/index.html`) flashed automatically to the `storage` partition. Wi-Fi credentials can also be stored dynamically in `/spiffs/wifi.txt`.

## Hardware & Wiring

| BME280 Pin | ESP32 DevKit V1 (Default) | ESP32-C3 SuperMini |
|------------|---------------------------|--------------------|
| **VCC**    | 3.3V                      | 3.3V               |
| **GND**    | GND                       | GND                |
| **SDA**    | GPIO 21                   | GPIO 8             |
| **SCL**    | GPIO 22                   | GPIO 9             |
| **SDO**    | GND (addr 0x76)           | GND (addr 0x76)    |

## Wi-Fi Configuration (No Hardcoded Secrets)

### Option 1: Header Configuration File
1. Copy the example configuration header:
   ```bash
   cp main/wifi_credentials.h.example main/wifi_credentials.h
   ```
2. Edit `main/wifi_credentials.h` with your SSID and password.
3. `wifi_credentials.h` is ignored by Git, preventing secret leakage.

### Option 2: SPIFFS Dynamic Storage
Place a file named `wifi.txt` in the SPIFFS partition or use the provisioning helper in `main.cpp`:
```
MY_WIFI_SSID;MY_WIFI_PASSWORD
```

## Compilation and Flashing (ESP-IDF)
```bash
# Set target chip (e.g., esp32, esp32c3, esp32s3)
idf.py set-target esp32

# Configure project (custom partitions are preset via sdkconfig.defaults)
idf.py menuconfig

# Build project and generate SPIFFS partition image
idf.py build

# Flash firmware and monitor serial output
idf.py -p COMx flash monitor
```
Replace `COMx` with your actual serial port (e.g. `/dev/ttyUSB0` on Linux, `COM3` on Windows).
