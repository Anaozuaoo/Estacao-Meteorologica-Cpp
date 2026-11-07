#include "NetworkManager.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <cstdio>
#include <cstring>

#if __has_include("wifi_credentials.h")
#include "wifi_credentials.h"
#endif

#ifndef CONFIG_WIFI_SSID
#define CONFIG_WIFI_SSID "YOUR_WIFI_SSID"
#endif

#ifndef CONFIG_WIFI_PASSWORD
#define CONFIG_WIFI_PASSWORD "YOUR_WIFI_PASSWORD"
#endif

static const char *TAG = "NetworkManager";

NetworkManager::NetworkManager(SpiffsManager& spiffsManager)
    : spiffs(spiffsManager), wifi_connected(false), server(nullptr) {
}

NetworkManager::~NetworkManager() {
    if (server) {
        httpd_stop(server);
        server = nullptr;
    }
}

esp_err_t NetworkManager::index_handler(httpd_req_t *req) {
    FILE* f = fopen("/spiffs/index.html", "r");
    if (f == nullptr) {
        ESP_LOGE(TAG, "index.html not found on SPIFFS");
        httpd_resp_send_404(req);
        return ESP_FAIL;
    }

    char line[128];
    while (fgets(line, sizeof(line), f)) {
        httpd_resp_send_chunk(req, line, HTTPD_RESP_USE_STRLEN);
    }
    fclose(f);
    httpd_resp_send_chunk(req, nullptr, 0);
    return ESP_OK;
}

esp_err_t NetworkManager::data_handler(httpd_req_t *req) {
    auto* self = static_cast<NetworkManager*>(req->user_ctx);

    char resp[128];
    snprintf(resp, sizeof(resp),
             "{\"temp\":%.3f,\"hum\":%.3f,\"pres\":%.3f}",
             self->currentData.temperature,
             self->currentData.humidity,
             self->currentData.pressure);

    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, resp, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

void NetworkManager::startWebServer() {
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.uri_match_fn = httpd_uri_match_wildcard;

    ESP_LOGI(TAG, "Starting HTTP Web Server on port %d...", config.server_port);
    if (httpd_start(&server, &config) == ESP_OK) {
        httpd_uri_t uri_index = {
            .uri       = "/",
            .method    = HTTP_GET,
            .handler   = index_handler,
            .user_ctx  = nullptr
        };
        httpd_register_uri_handler(server, &uri_index);

        httpd_uri_t uri_data = {
            .uri       = "/api/dados",
            .method    = HTTP_GET,
            .handler   = data_handler,
            .user_ctx  = this
        };
        httpd_register_uri_handler(server, &uri_data);

        ESP_LOGI(TAG, "Web Server routes registered: GET / and GET /api/dados");
    } else {
        ESP_LOGE(TAG, "Failed to start HTTP Web Server!");
    }
}

void NetworkManager::updateData(const SensorData& data) {
    currentData = data;
}

void NetworkManager::connect() {
    // 1. Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // 2. Initialize TCP/IP and Event Loop
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    // 3. Resolve Credentials (SPIFFS first, then compile-time config)
    WifiCreds creds = spiffs.readCredentials();
    std::string active_ssid;
    std::string active_pass;

    if (!creds.ssid.empty()) {
        active_ssid = creds.ssid;
        active_pass = creds.pass;
        ESP_LOGI(TAG, "Using credentials from SPIFFS: SSID='%s'", active_ssid.c_str());
    } else {
        active_ssid = CONFIG_WIFI_SSID;
        active_pass = CONFIG_WIFI_PASSWORD;
        ESP_LOGI(TAG, "Using credentials from configuration header: SSID='%s'", active_ssid.c_str());
    }

    if (active_ssid == "YOUR_WIFI_SSID" || active_ssid.empty()) {
        ESP_LOGW(TAG, "==========================================================");
        ESP_LOGW(TAG, "ATTENTION: Wi-Fi credentials not configured!");
        ESP_LOGW(TAG, "Configure them via 'wifi_credentials.h' or '/spiffs/wifi.txt'");
        ESP_LOGW(TAG, "==========================================================");
    }

    wifi_config_t wifi_config = {};
    std::strncpy(reinterpret_cast<char*>(wifi_config.sta.ssid), active_ssid.c_str(), sizeof(wifi_config.sta.ssid) - 1);
    std::strncpy(reinterpret_cast<char*>(wifi_config.sta.password), active_pass.c_str(), sizeof(wifi_config.sta.password) - 1);
    wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "Connecting to Wi-Fi station '%s'...", active_ssid.c_str());
    esp_wifi_connect();

    // Allow time for DHCP negotiation
    vTaskDelay(pdMS_TO_TICKS(5000));

    startWebServer();
    wifi_connected = true;
    ESP_LOGI(TAG, "Station active. Check device IP in serial logs to access dashboard.");
}
