#include "azuracast_client.h"
#include "config.h"

#ifndef DESKTOP_TEST
#include <WiFi.h>
#include <WiFiSSLClient.h>
#endif

#include <ArduinoHttpClient.h>
#include <ArduinoJson.h>
#include <Client.h>

AzuraCastClient::AzuraCastClient(const char* host, int port, const char* path)
    : host(host)
    , port(port)
    , path(path)
    , lastShId(0)
    , liveDJ(false)
{
}

bool AzuraCastClient::poll(Client& client) {
    HttpClient http(client, host, port);
    http.setHttpResponseTimeout(HTTP_RESPONSE_TIMEOUT_MS);

    Serial.print("[AzuraCast] Polling...");
    int err = http.get(path);
    if (err != 0) {
        Serial.print(" connection error: ");
        Serial.println(err);
        http.stop();
        return false;
    }

    int statusCode = http.responseStatusCode();
    if (statusCode != 200) {
        Serial.print(" HTTP ");
        Serial.println(statusCode);
        // Must fully read response body before stop() to avoid socket leak
        http.responseBody();
        http.stop();
        return false;
    }

    // Filter document: parse only the fields we need from the ~10KB response.
    // This keeps ArduinoJson memory usage under 1KB.
    JsonDocument filter;
    filter["now_playing"]["sh_id"] = true;
    filter["now_playing"]["song"]["artist"] = true;
    filter["now_playing"]["song"]["title"] = true;
    filter["now_playing"]["song"]["album"] = true;
    filter["live"]["is_live"] = true;

    // Read body as String, then parse with filter. HttpClient does not extend
    // Stream, so we buffer the body rather than streaming directly.
    String body = http.responseBody();
    http.stop();

    JsonDocument doc;
    DeserializationError jsonErr = deserializeJson(doc, body,
        DeserializationOption::Filter(filter));

    if (jsonErr) {
        Serial.print(" JSON parse error: ");
        Serial.println(jsonErr.c_str());
        return false;
    }

    liveDJ = doc["live"]["is_live"] | false;

    int shId = doc["now_playing"]["sh_id"] | 0;
    if (shId == 0) {
        Serial.println(" no sh_id in response.");
        return false;
    }

    if (shId == lastShId) {
        Serial.println(" same track.");
        return false;
    }

    // New track detected
    lastShId = shId;
    artist = doc["now_playing"]["song"]["artist"].as<String>();
    title = doc["now_playing"]["song"]["title"].as<String>();
    album = doc["now_playing"]["song"]["album"].as<String>();

    Serial.print(" new track: ");
    Serial.print(artist);
    Serial.print(" - ");
    Serial.println(title);

    return true;
}

#ifndef DESKTOP_TEST
bool AzuraCastClient::poll() {
    // Create SSL client locally to avoid the Giga R1 global WiFiClient crash
    // bug. Must call stop() before going out of scope to prevent socket leak.
    WiFiSSLClient ssl;
    return poll(ssl);
}
#endif

String AzuraCastClient::getArtist() const { return artist; }
String AzuraCastClient::getTitle() const { return title; }
String AzuraCastClient::getAlbum() const { return album; }
int AzuraCastClient::getShId() const { return lastShId; }
bool AzuraCastClient::isLiveDJ() const { return liveDJ; }
