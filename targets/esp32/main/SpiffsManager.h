#ifndef SPIFFSMANAGER_H
#define SPIFFSMANAGER_H

#include <string>

/**
 * @brief Credentials data structure for Wi-Fi station connectivity.
 */
struct WifiCreds {
    std::string ssid;
    std::string pass;
};

/**
 * @brief Manages SPIFFS filesystem partition initialization and credential persistence.
 */
class SpiffsManager {
public:
    SpiffsManager();

    /**
     * @brief Read credentials from /spiffs/wifi.txt.
     * @return WifiCreds containing loaded SSID and Password (empty if file does not exist).
     */
    WifiCreds readCredentials();

    /**
     * @brief Save credentials into /spiffs/wifi.txt.
     * @param creds WifiCreds to store.
     * @return true on success, false on error.
     */
    bool writeCredentials(const WifiCreds& creds);

    /**
     * @brief Check whether SPIFFS was mounted successfully.
     */
    bool isMounted() const { return mounted; }

private:
    bool mounted = false;
};

#endif // SPIFFSMANAGER_H
