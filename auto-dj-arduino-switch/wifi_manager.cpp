#include "wifi_manager.h"

WifiManager::WifiManager(const char* ssid, const char* password, unsigned long retryIntervalMs)
    : ssid(ssid)
    , password(password)
    , retryIntervalMs(retryIntervalMs)
    , lastRetryTime(0)
    , wasConnected(false)
{
}

void WifiManager::setUp() {
    Serial.print("[WiFi] MAC address: ");
    Serial.println(WiFi.macAddress());
    Serial.print("[WiFi] Connecting to ");
    Serial.print(ssid);
    Serial.print("...");

    WiFi.begin(ssid, password);

    // Block until connected on initial setup
    unsigned long start = millis();
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
        if (millis() - start > 30000) {
            Serial.println(" timeout.");
            return;
        }
    }

    Serial.println(" connected.");
    Serial.print("[WiFi] IP: ");
    Serial.println(WiFi.localIP());
    wasConnected = true;
}

void WifiManager::update() {
    bool connected = (WiFi.status() == WL_CONNECTED);

    if (wasConnected && !connected) {
        Serial.println("[WiFi] Connection lost.");
        wasConnected = false;
    }

    if (!connected && (millis() - lastRetryTime > retryIntervalMs)) {
        lastRetryTime = millis();
        Serial.print("[WiFi] Reconnecting...");
        WiFi.disconnect();
        delay(100);
        WiFi.begin(ssid, password);

        // Brief wait to check if connection succeeds quickly
        unsigned long start = millis();
        while (WiFi.status() != WL_CONNECTED && millis() - start < 5000) {
            delay(250);
        }

        if (WiFi.status() == WL_CONNECTED) {
            Serial.println(" reconnected.");
            Serial.print("[WiFi] IP: ");
            Serial.println(WiFi.localIP());
            wasConnected = true;
        } else {
            Serial.println(" still disconnected.");
        }
    }
}

bool WifiManager::isConnected() const {
    return WiFi.status() == WL_CONNECTED;
}

String WifiManager::macAddress() const {
    return WiFi.macAddress();
}

unsigned long WifiManager::getEpochTime() {
    return WiFi.getTime();
}
