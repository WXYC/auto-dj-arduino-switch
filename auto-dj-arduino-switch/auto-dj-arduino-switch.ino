/**
 * Auto DJ Arduino Switch
 *
 * Monitors the WXYC mixing board's AUX relay contact to detect when the
 * auto DJ system (AzuraCast/Liquidsoap at remote.wxyc.org) is active.
 * When active, polls AzuraCast for currently-playing track data and writes
 * entries to the tubafrenzy flowsheet (wxyc.info), bridging the gap in
 * the station's playback history.
 *
 * Hardware: Arduino Giga R1 WiFi
 * Wiring:   See docs/wiring.md
 */

#include "config.h"
#include "secrets.h"
#include "relay_monitor.h"
#include "wifi_manager.h"
#include "azuracast_client.h"
#include "flowsheet_client.h"

// ========== State Machine ==========

enum State {
    BOOTING,
    CONNECTING_WIFI,
    IDLE,
    STARTING_SHOW,
    AUTO_DJ_ACTIVE,
    ENDING_SHOW,
    ERROR_STATE
};

const char* stateNames[] = {
    "BOOTING", "CONNECTING_WIFI", "IDLE", "STARTING_SHOW",
    "AUTO_DJ_ACTIVE", "ENDING_SHOW", "ERROR"
};

// ========== Global State ==========

State currentState = BOOTING;
int radioShowID = -1;
unsigned long lastPollTime = 0;
unsigned long lastNtpSync = 0;
int retryCount = 0;

// ========== Modules ==========

RelayMonitor relayMonitor(RELAY_PIN, STATUS_LED_PIN, DEBOUNCE_MS);
WifiManager wifiManager(WIFI_SSID, WIFI_PASS, WIFI_RETRY_INTERVAL_MS);
AzuraCastClient azuracast(AZURACAST_HOST, AZURACAST_PORT, AZURACAST_PATH);
FlowsheetClient flowsheet(TUBAFRENZY_HOST, TUBAFRENZY_PORT, AUTO_DJ_API_KEY);

// ========== Time Helpers ==========

/**
 * Returns the current hour's epoch milliseconds (truncated to hour boundary).
 * Used for startingHour and workingHour parameters.
 */
unsigned long currentHourMs() {
    unsigned long epoch = wifiManager.getEpochTime();
    if (epoch == 0) return 0;
    // Truncate to hour boundary: subtract seconds-within-hour, convert to ms
    unsigned long hourEpoch = epoch - (epoch % 3600);
    return hourEpoch * 1000UL;
}

// ========== State Transitions ==========

void transitionTo(State newState) {
    Serial.print("[State] ");
    Serial.print(stateNames[currentState]);
    Serial.print(" -> ");
    Serial.println(stateNames[newState]);
    currentState = newState;
    retryCount = 0;
}

// ========== Setup ==========

void setup() {
    Serial.begin(115200);
    while (!Serial && millis() < 3000); // Wait up to 3s for Serial
    Serial.println();
    Serial.println("=== WXYC Auto DJ Arduino Switch ===");

    pinMode(LED_BUILTIN, OUTPUT);
    relayMonitor.setUp();

    transitionTo(CONNECTING_WIFI);
    wifiManager.setUp();

    if (wifiManager.isConnected()) {
        lastNtpSync = millis();
        Serial.print("[Time] Epoch: ");
        Serial.println(wifiManager.getEpochTime());
        transitionTo(IDLE);
    }
}

// ========== Main Loop ==========

void loop() {
    // Always update hardware monitors
    relayMonitor.update();
    wifiManager.update();

    // Heartbeat LED
    digitalWrite(LED_BUILTIN, (millis() / 1000) % 2 == 0 ? HIGH : LOW);

    // Periodic NTP re-sync
    if (wifiManager.isConnected() && (millis() - lastNtpSync > NTP_SYNC_INTERVAL_MS)) {
        lastNtpSync = millis();
        Serial.print("[Time] NTP re-sync, epoch: ");
        Serial.println(wifiManager.getEpochTime());
    }

    // WiFi loss handling: if we lose WiFi in any state, go to CONNECTING_WIFI
    // but preserve radioShowID so we can resume after reconnection.
    if (currentState != BOOTING && currentState != CONNECTING_WIFI && !wifiManager.isConnected()) {
        Serial.println("[State] WiFi lost, transitioning to CONNECTING_WIFI.");
        currentState = CONNECTING_WIFI;
        return;
    }

    switch (currentState) {
        case BOOTING:
            // Should not stay here -- setup() transitions out
            break;

        case CONNECTING_WIFI:
            if (wifiManager.isConnected()) {
                // If we had an active show before WiFi dropped, resume it
                if (radioShowID > 0) {
                    Serial.println("[State] WiFi restored, resuming active show.");
                    transitionTo(AUTO_DJ_ACTIVE);
                } else {
                    transitionTo(IDLE);
                }
            }
            break;

        case IDLE:
            if (relayMonitor.stateChanged() && relayMonitor.isAutoDJActive()) {
                transitionTo(STARTING_SHOW);
            }
            break;

        case STARTING_SHOW: {
            unsigned long hourMs = currentHourMs();
            if (hourMs == 0) {
                Serial.println("[Error] No NTP time available, cannot start show.");
                transitionTo(ERROR_STATE);
                break;
            }

            int showID = flowsheet.startShow(hourMs);
            if (showID > 0) {
                radioShowID = showID;
                lastPollTime = 0; // Force immediate first poll
                transitionTo(AUTO_DJ_ACTIVE);
            } else {
                retryCount++;
                if (retryCount >= MAX_RETRIES) {
                    Serial.println("[Error] Failed to start show after max retries.");
                    transitionTo(ERROR_STATE);
                } else {
                    Serial.print("[Retry] Start show attempt ");
                    Serial.print(retryCount);
                    Serial.print("/");
                    Serial.println(MAX_RETRIES);
                    delay(RETRY_BACKOFF_MS * retryCount);
                }
            }
            break;
        }

        case AUTO_DJ_ACTIVE:
            // Check if DJ has taken over
            if (relayMonitor.stateChanged() && !relayMonitor.isAutoDJActive()) {
                transitionTo(ENDING_SHOW);
                break;
            }

            // Poll AzuraCast on interval
            if (millis() - lastPollTime >= POLL_INTERVAL_MS) {
                lastPollTime = millis();

                bool newTrack = azuracast.poll();

                // If a live DJ is streaming to AzuraCast, skip flowsheet entries
                // (the relay is the primary source of truth, but this is a safety check)
                if (newTrack && !azuracast.isLiveDJ()) {
                    unsigned long hourMs = currentHourMs();
                    if (hourMs > 0) {
                        flowsheet.addEntry(radioShowID, hourMs,
                            azuracast.getArtist(),
                            azuracast.getTitle(),
                            azuracast.getAlbum());
                    }
                }
            }
            break;

        case ENDING_SHOW: {
            bool ended = flowsheet.endShow(radioShowID);
            if (ended) {
                radioShowID = -1;
                transitionTo(IDLE);
            } else {
                retryCount++;
                if (retryCount >= MAX_RETRIES) {
                    Serial.println("[Warning] Failed to end show after max retries. Returning to IDLE.");
                    radioShowID = -1;
                    transitionTo(IDLE);
                } else {
                    delay(RETRY_BACKOFF_MS * retryCount);
                }
            }
            break;
        }

        case ERROR_STATE:
            // Wait and retry -- check if conditions have improved
            if (!wifiManager.isConnected()) {
                transitionTo(CONNECTING_WIFI);
            } else if (relayMonitor.isAutoDJActive() && radioShowID <= 0) {
                // Retry starting the show
                transitionTo(STARTING_SHOW);
            } else if (!relayMonitor.isAutoDJActive()) {
                transitionTo(IDLE);
            }
            delay(RETRY_BACKOFF_MS);
            break;
    }
}
