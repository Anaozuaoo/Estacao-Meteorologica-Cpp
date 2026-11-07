idf_component_register(
    SRCS
        "main.cpp"
        "DeviceController.cpp"
        "NetworkManager.cpp"
        "SpiffsManager.cpp"
        "ESP32I2CDriver.cpp"
        "../../../core/src/BME280Sensor.cpp"
    INCLUDE_DIRS
        "."
        "../../../core/include"
    REQUIRES
        driver
        esp_timer
        spiffs
        nvs_flash
        esp_wifi
        esp_event
        esp_http_server
)

# Flash the web dashboard located in site_data into the SPIFFS 'storage' partition
spiffs_create_partition_image(storage site_data FLASH_IN_PROJECT)
