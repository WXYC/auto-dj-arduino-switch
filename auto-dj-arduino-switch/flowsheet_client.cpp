#include "flowsheet_client.h"
#include "config.h"

#include <WiFi.h>
#include <WiFiSSLClient.h>
#include <ArduinoHttpClient.h>

FlowsheetClient::FlowsheetClient(const char* host, int port, const char* apiKey)
    : host(host)
    , port(port)
    , apiKey(apiKey)
{
}

// ========== URL Encoding ==========

String FlowsheetClient::urlEncode(const String& str) {
    String encoded;
    encoded.reserve(str.length() * 2);
    for (unsigned int i = 0; i < str.length(); i++) {
        char c = str.charAt(i);
        if (isAlphaNumeric(c) || c == '-' || c == '_' || c == '.' || c == '~') {
            encoded += c;
        } else if (c == ' ') {
            encoded += '+';
        } else {
            encoded += '%';
            if ((unsigned char)c < 0x10) encoded += '0';
            encoded += String((unsigned char)c, HEX);
        }
    }
    return encoded;
}

// ========== HTTP Helpers ==========

/**
 * POSTs a form-encoded body and returns the HTTP status code, or -1 on error.
 * Fully reads the response body and stops the client to prevent socket leaks.
 */
int FlowsheetClient::postForm(const char* path, const String& body) {
    WiFiSSLClient ssl;
    HttpClient http(ssl, host, port);
    http.setHttpResponseTimeout(HTTP_RESPONSE_TIMEOUT_MS);

    http.beginRequest();
    http.post(path);
    http.sendHeader("Content-Type", "application/x-www-form-urlencoded");
    http.sendHeader("Content-Length", body.length());
    http.sendHeader("X-Auto-DJ-Key", apiKey);
    http.beginBody();
    http.print(body);
    http.endRequest();

    int statusCode = http.responseStatusCode();
    http.responseBody(); // drain response to prevent socket leak
    http.stop();
    return statusCode;
}

/**
 * POSTs a form-encoded body and returns the Location header from a 302 redirect.
 * Returns empty string on failure.
 */
String FlowsheetClient::getLocationHeader(const char* path, const String& body) {
    WiFiSSLClient ssl;
    HttpClient http(ssl, host, port);
    http.setHttpResponseTimeout(HTTP_RESPONSE_TIMEOUT_MS);

    http.beginRequest();
    http.post(path);
    http.sendHeader("Content-Type", "application/x-www-form-urlencoded");
    http.sendHeader("Content-Length", body.length());
    http.sendHeader("X-Auto-DJ-Key", apiKey);
    http.beginBody();
    http.print(body);
    http.endRequest();

    int statusCode = http.responseStatusCode();

    // Read the Location header before draining the body
    String location;
    while (http.headerAvailable()) {
        String headerName = http.readHeaderName();
        String headerValue = http.readHeaderValue();
        if (headerName.equalsIgnoreCase("Location")) {
            location = headerValue;
        }
    }

    http.responseBody(); // drain
    http.stop();

    if (statusCode != 302) {
        Serial.print("[Flowsheet] Expected 302, got ");
        Serial.println(statusCode);
        return "";
    }

    return location;
}

// ========== Public API ==========

int FlowsheetClient::startShow(unsigned long startingHourMs) {
    Serial.println("[Flowsheet] Starting show...");

    String body = "djID=" + String(AUTO_DJ_ID)
        + "&djName=" + urlEncode(AUTO_DJ_NAME)
        + "&djHandle=" + urlEncode(AUTO_DJ_HANDLE)
        + "&showName=" + urlEncode(AUTO_DJ_SHOW_NAME)
        + "&startingHour=" + String(startingHourMs);

    String location = getLocationHeader(TUBAFRENZY_PATH_START_SHOW, body);
    if (location.length() == 0) {
        Serial.println("[Flowsheet] Failed to start show (no Location header).");
        return -1;
    }

    // Parse radioShowID from Location: /playlists/flowsheet?mode=modifyFlowsheet&radioShowID=123
    int idx = location.indexOf("radioShowID=");
    if (idx < 0) {
        Serial.print("[Flowsheet] radioShowID not found in Location: ");
        Serial.println(location);
        return -1;
    }

    String idStr = location.substring(idx + 12); // length of "radioShowID="
    // Trim any trailing parameters
    int ampIdx = idStr.indexOf('&');
    if (ampIdx >= 0) {
        idStr = idStr.substring(0, ampIdx);
    }

    int radioShowID = idStr.toInt();
    if (radioShowID <= 0) {
        Serial.print("[Flowsheet] Failed to parse radioShowID from: ");
        Serial.println(idStr);
        return -1;
    }

    Serial.print("[Flowsheet] Show started, radioShowID=");
    Serial.println(radioShowID);
    return radioShowID;
}

bool FlowsheetClient::addEntry(int radioShowID, unsigned long workingHourMs,
                                const String& artist, const String& title,
                                const String& album) {
    Serial.print("[Flowsheet] Adding entry: ");
    Serial.print(artist);
    Serial.print(" - ");
    Serial.println(title);

    String body = "radioShowID=" + String(radioShowID)
        + "&workingHour=" + String(workingHourMs)
        + "&artistName=" + urlEncode(artist)
        + "&songTitle=" + urlEncode(title)
        + "&releaseTitle=" + urlEncode(album)
        + "&releaseType=otherRelease"
        + "&autoBreakpoint=true";

    int status = postForm(TUBAFRENZY_PATH_ADD_ENTRY, body);
    if (status == 302) {
        Serial.println("[Flowsheet] Entry added.");
        return true;
    }

    Serial.print("[Flowsheet] Failed to add entry, HTTP ");
    Serial.println(status);
    return false;
}

bool FlowsheetClient::endShow(int radioShowID) {
    Serial.println("[Flowsheet] Ending show...");

    String body = "radioShowID=" + String(radioShowID)
        + "&mode=signoffConfirm";

    int status = postForm(TUBAFRENZY_PATH_END_SHOW, body);
    if (status == 302) {
        Serial.print("[Flowsheet] Show ended, radioShowID=");
        Serial.println(radioShowID);
        return true;
    }

    Serial.print("[Flowsheet] Failed to end show, HTTP ");
    Serial.println(status);
    return false;
}
