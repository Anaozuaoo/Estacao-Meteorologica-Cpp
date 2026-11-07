#include "DeviceController.h"
#include "SpiffsManager.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char* TAG = "MAIN";

extern "C" void app_main(void) {
    ESP_LOGI(TAG, "==================================================");
    ESP_LOGI(TAG, "=== IoT Weather Station - Embedded System ===");
    ESP_LOGI(TAG, "=== Target: ESP32                             ===");
    ESP_LOGI(TAG, "==================================================");

    /*
     * OPTIONAL: INITIAL SETUP TO PROVISION WI-FI CREDENTIALS IN FLASH (SPIFFS)
     * If you want to store your Wi-Fi credentials directly in flash memory:
     * 1. Uncomment the block below.
     * 2. Replace "YOUR_SSID" and "YOUR_PASSWORD" with your network details.
     * 3. Flash and run once.
     * 4. Re-comment or clean this section so credentials are not in source code.
     *
     * SpiffsManager sm;
     * WifiCreds creds;
     * creds.ssid = "YOUR_SSID";
     * creds.pass = "YOUR_PASSWORD";
     * if (sm.writeCredentials(creds)) {
     *     ESP_LOGI(TAG, "Credentials saved to /spiffs/wifi.txt! Restarting in 3s...");
     *     vTaskDelay(pdMS_TO_TICKS(3000));
     *     esp_restart();
     * }
     */

    // Start main orchestrator (instantiates drivers, mounts SPIFFS, starts server & RTOS tasks)
    static DeviceController controller;
    controller.start();

    ESP_LOGI(TAG, "System running. app_main entering idle task loop.");

    while (true) {
        vTaskDelay(pdMS_TO_TICKS(10000));
    }
}
