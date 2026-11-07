#ifndef NETWORKMANAGER_H
#define NETWORKMANAGER_H

#include "SpiffsManager.h"
#include "SensorBase.h"
#include "esp_http_server.h"
#include "esp_event.h"

/**
 * @brief Manages Wi-Fi connection and native HTTP Web Server for telemetry.
 */
class NetworkManager {
public:
    explicit NetworkManager(SpiffsManager& spiffsManager);
    ~NetworkManager();

    /**
     * @brief Connect to Wi-Fi station and initialize embedded web server.
     */
    void connect();

    /**
     * @brief Update the environmental readings served via REST API (/api/dados).
     */
    void updateData(const SensorData& data);

    /**
     * @brief Check if Wi-Fi has acquired an IP address.
     */
    bool isConnected() const { return wifi_connected; }

private:
    SpiffsManager& spiffs;
    bool wifi_connected;
    httpd_handle_t server = nullptr;

    SensorData currentData;

    void startWebServer();

    // HTTP Route Handlers
    static esp_err_t index_handler(httpd_req_t *req);
    static esp_err_t data_handler(httpd_req_t *req);
};

#endif // NETWORKMANAGER_H
