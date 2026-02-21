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
 *
 * Architecture: The loop() function is a thin orchestrator that performs I/O
 * and delegates all transition/retry logic to the pure tick() function in
 * state_machine.h. See test/test_state_machine.cpp for the full test suite.
 */

#include "config.h"
#include "secrets.h"
#include "relay_monitor.h"
#include "wifi_manager.h"
#include "azuracast_client.h"
#include "flowsheet_client.h"
#include "utils.h"
#include "state_machine.h"

// ========== Global State ==========

Context ctx = { BOOTING, -1, 0, 0 };
unsigned long lastNtpSync = 0;

// ========== Modules ==========

RelayMonitor relayMonitor(RELAY_PIN, STATUS_LED_PIN, DEBOUNCE_MS);
WifiManager wifiManager(WIFI_SSID, WIFI_PASS, WIFI_RETRY_INTERVAL_MS);
AzuraCastClient azuracast(AZURACAST_HOST, AZURACAST_PORT, AZURACAST_PATH);
FlowsheetClient flowsheet(TUBAFRENZY_HOST, TUBAFRENZY_PORT, AUTO_DJ_API_KEY);

// ========== Logging ==========

void logTransition(State prev, State next) {
    if (prev != next) {
        Serial.print("[State] ");
        Serial.print(stateName(prev));
        Serial.print(" -> ");
        Serial.println(stateName(next));
    }
}

// ========== Setup ==========

void setup() {
    Serial.begin(115200);
    while (!Serial && millis() < 3000); // Wait up to 3s for Serial
    Serial.println();
    Serial.println("=== WXYC Auto DJ Arduino Switch ===");

    pinMode(LED_BUILTIN, OUTPUT);
    relayMonitor.setUp();

    ctx.state = CONNECTING_WIFI;
    ctx.retryCount = 0;
    Serial.print("[State] BOOTING -> CONNECTING_WIFI");
    Serial.println();

    wifiManager.setUp();

    if (wifiManager.isConnected()) {
        lastNtpSync = millis();
        Serial.print("[Time] Epoch: ");
        Serial.println(wifiManager.getEpochTime());
        ctx.state = IDLE;
        ctx.retryCount = 0;
        Serial.println("[State] CONNECTING_WIFI -> IDLE");
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

    // ---- GATHER INPUTS ----
    Inputs inputs;
    inputs.relayStateChanged = relayMonitor.stateChanged();
    inputs.autoDJActive = relayMonitor.isAutoDJActive();
    inputs.wifiConnected = wifiManager.isConnected();
    inputs.epochTime = wifiManager.getEpochTime();
    inputs.currentMillis = millis();
    inputs.pollIntervalMs = POLL_INTERVAL_MS;
    inputs.maxRetries = MAX_RETRIES;
    inputs.retryBackoffMs = RETRY_BACKOFF_MS;

    // Default I/O results
    inputs.startShowResult = -1;
    inputs.endShowResult = false;
    inputs.pollNewTrack = false;
    inputs.pollLiveDJ = false;

    // ---- PRE-TICK I/O ----
    switch (ctx.state) {
        case STARTING_SHOW: {
            unsigned long hourMs = currentHourMs(inputs.epochTime);
            if (hourMs > 0) {
                inputs.startShowResult = flowsheet.startShow(hourMs);
            }
            break;
        }
        case AUTO_DJ_ACTIVE:
            if (inputs.currentMillis - ctx.lastPollTime >= POLL_INTERVAL_MS) {
                inputs.pollNewTrack = azuracast.poll();
                inputs.pollLiveDJ = azuracast.isLiveDJ();
                inputs.artist = azuracast.getArtist();
                inputs.title = azuracast.getTitle();
                inputs.album = azuracast.getAlbum();
            }
            break;
        case ENDING_SHOW:
            inputs.endShowResult = flowsheet.endShow(ctx.radioShowID);
            break;
        default:
            break;
    }

    // ---- TICK ----
    State prevState = ctx.state;
    TickResult result = tick(ctx, inputs);
    ctx = result.context;
    logTransition(prevState, ctx.state);

    // ---- POST-TICK I/O ----
    if (result.addEntry) {
        flowsheet.addEntry(ctx.radioShowID, result.addEntryHourMs,
            result.addEntryArtist, result.addEntryTitle, result.addEntryAlbum);
    }
    if (result.delayMs > 0) {
        delay(result.delayMs);
    }
}
