#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <Arduino.h>
#include <WiFi.h>

/**
 * Manages WiFi connection with automatic reconnection.
 *
 * Note: WiFi.begin() blocks for ~36 seconds on reconnection (known Giga R1
 * limitation). The state machine in the main sketch should account for this.
 */
class WifiManager {
public:
    WifiManager(const char* ssid, const char* password, unsigned long retryIntervalMs);
    void setUp();
    void update();
    bool isConnected() const;
    String macAddress() const;
    unsigned long getEpochTime();

private:
    const char* ssid;
    const char* password;
    unsigned long retryIntervalMs;
    unsigned long lastRetryTime;
    bool wasConnected;
};

#endif
