#include "SpiffsManager.h"
#include "esp_log.h"
#include "esp_spiffs.h"
#include <cstdio>
#include <cstring>

static const char* TAG = "SpiffsManager";
static const char* CREDENTIALS_FILE = "/spiffs/wifi.txt";

SpiffsManager::SpiffsManager() {
    ESP_LOGI(TAG, "Initializing SPIFFS...");
    esp_vfs_spiffs_conf_t conf = {
        .base_path = "/spiffs",
        .partition_label = "storage",
        .max_files = 4,
        .format_if_mount_failed = true
    };

    esp_err_t ret = esp_vfs_spiffs_register(&conf);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to mount SPIFFS (%s). Ensure partitions.csv defines 'storage' partition!", esp_err_to_name(ret));
        mounted = false;
        return;
    }

    size_t total = 0, used = 0;
    ret = esp_spiffs_info("storage", &total, &used);
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "SPIFFS mounted: total %d bytes, used %d bytes", (int)total, (int)used);
    }
    mounted = true;
}

WifiCreds SpiffsManager::readCredentials() {
    WifiCreds creds;
    if (!mounted) {
        return creds;
    }

    FILE* f = fopen(CREDENTIALS_FILE, "r");
    if (f == nullptr) {
        ESP_LOGI(TAG, "No credentials file found at %s", CREDENTIALS_FILE);
        return creds;
    }

    char line[128];
    if (fgets(line, sizeof(line), f) != nullptr) {
        char* newline = strchr(line, '\r');
        if (newline) *newline = '\0';
        newline = strchr(line, '\n');
        if (newline) *newline = '\0';

        char* ssid_token = strtok(line, ";");
        char* pass_token = strtok(nullptr, ";");
        if (ssid_token && pass_token) {
            creds.ssid = std::string(ssid_token);
            creds.pass = std::string(pass_token);
            ESP_LOGI(TAG, "Read Wi-Fi credentials from storage: SSID=%s", creds.ssid.c_str());
        }
    }
    fclose(f);
    return creds;
}

bool SpiffsManager::writeCredentials(const WifiCreds& creds) {
    if (!mounted) {
        return false;
    }

    FILE* f = fopen(CREDENTIALS_FILE, "w");
    if (f == nullptr) {
        ESP_LOGE(TAG, "Failed to open %s for writing", CREDENTIALS_FILE);
        return false;
    }

    int bytes_written = fprintf(f, "%s;%s\n", creds.ssid.c_str(), creds.pass.c_str());
    fclose(f);
    return (bytes_written > 0);
}
