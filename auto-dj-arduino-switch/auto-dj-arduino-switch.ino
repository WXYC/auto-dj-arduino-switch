/**
 * Auto DJ Arduino Switch -- relay/button reporter
 *
 * Reports the mixing board's AUX relay state and a manual toggle button to the
 * auto-dj-orchestrator over a WebSocket management channel (HTTP fallback over
 * WiFi). The orchestrator owns all activation logic and flowsheet writes; this
 * board only reports inputs and drives the status LED.
 *
 * Hardware: Arduino Giga R1 WiFi (+ Ethernet Shield Rev2, Phase 2)
 * Wiring:   See docs/wiring.md
 *
 * Architecture: loop() is a thin I/O orchestrator that delegates all decision
 * logic to the pure tick() in state_machine.h and all JSON to mgmt_protocol.h.
 * See test/ for the desktop test suite.
 */

#include "config.h"
#include "secrets.h"
#include "relay_monitor.h"
#include "button_monitor.h"
#include "wifi_manager.h"
#include "state_machine.h"
#include "mgmt_protocol.h"
#include "mgmt_client.h"

// ========== Global state ==========

Context ctx = { BOOTING, false, false, TRANSPORT_NONE, false, 0, 0 };

// Telemetry
unsigned int reconnectCount = 0;
unsigned int buttonPressCount = 0;  // HTTP-fallback parity; reset after each heartbeat
unsigned long loopMaxMs = 0;

// ========== Modules ==========

RelayMonitor relayMonitor(RELAY_PIN, STATUS_LED_PIN, DEBOUNCE_MS);
ButtonMonitor buttonMonitor(BUTTON_PIN, DEBOUNCE_MS);
WifiManager wifiManager(WIFI_SSID, WIFI_PASS, WIFI_RETRY_INTERVAL_MS);
MgmtClient mgmt(ORCHESTRATOR_HOST, ORCHESTRATOR_PORT, ORCHESTRATOR_WS_PATH,
                ORCHESTRATOR_HB_PATH, ORCHESTRATOR_CMD_PATH, AUTO_DJ_KEY, ORCHESTRATOR_USE_TLS);

// ========== Helpers ==========

void logTransition(State prev, State next) {
    if (prev != next) {
        Serial.print("[State] ");
        Serial.print(stateName(prev));
        Serial.print(" -> ");
        Serial.println(stateName(next));
    }
}

HeartbeatFields makeHeartbeat(const Context& c, const Inputs& in) {
    HeartbeatFields f;
    f.state = stateName(c.state);
    f.transport = c.transport == TRANSPORT_ETHERNET ? "ethernet" : "wifi";
    f.uptime_s = in.currentMillis / 1000;
    f.wifi_rssi_null = c.transport == TRANSPORT_ETHERNET;
    f.wifi_rssi = (long)WiFi.RSSI();
    f.free_ram = 0;  // Giga/Mbed: heap stats not reported here (telemetry only)
    f.firmware_version = FIRMWARE_VERSION;
    f.config_hash = "";
    f.loop_max_ms = loopMaxMs;
    f.reconnect_count = reconnectCount;
    f.tracks_detected = 0;  // reporter model: orchestrator owns track detection
    f.tracks_posted = 0;
    f.errors_since_boot = 0;
    f.button_press_count = buttonPressCount;
    f.relay_auto_dj_active = c.relayAutoDJActive;
    return f;
}

// ========== Setup ==========

void setup() {
    Serial.begin(115200);
    while (!Serial && millis() < 3000);
    Serial.println();
    Serial.println("=== WXYC Auto DJ Reporter ===");

    pinMode(STATUS_LED_PIN, OUTPUT);
    relayMonitor.setUp();
    buttonMonitor.setUp();
    wifiManager.setUp();

    if (wifiManager.isConnected() && mgmt.connectWs()) {
        reconnectCount++;
    }
}

// ========== Main loop ==========

void loop() {
    unsigned long loopStart = millis();

    relayMonitor.update();
    buttonMonitor.update();
    wifiManager.update();
    mgmt.poll();

    bool wifiUp = wifiManager.isConnected();
    bool channelUp = mgmt.channelUp();

    // Reconnect the management channel if the network is up but the channel dropped.
    if (wifiUp && !channelUp) {
        if (mgmt.connectWs()) {
            reconnectCount++;
            channelUp = mgmt.channelUp();
        }
    }

    // ---- Gather inputs ----
    Inputs in;
    in.relayChanged = relayMonitor.stateChanged();
    in.relayAutoDJActive = relayMonitor.isAutoDJActive();
    in.buttonPressed = buttonMonitor.pressed();
    in.ethernetLinkUp = false;  // Ethernet shield: Phase 2
    in.wifiConnected = wifiUp;
    in.channelUp = channelUp;

    String cmdId;
    in.gotCommand = mgmt.takeCommand(&in.commandAction, &cmdId);
    if (!in.gotCommand) in.commandAction = CMD_NONE;
    in.gotActiveResult = mgmt.takeActiveResult(&in.activeResult);

    in.currentMillis = millis();
    in.heartbeatIntervalMs = WS_HEARTBEAT_MS;
    in.retryBackoffMs = RETRY_BACKOFF_MS;
    in.maxRetries = MAX_RETRIES;

    if (in.buttonPressed) buttonPressCount++;

    // ---- Tick (pure) ----
    State prev = ctx.state;
    TickResult r = tick(ctx, in);
    ctx = r.context;
    logTransition(prev, ctx.state);

    // ---- Post-tick I/O (ordered) ----
    if (r.sendAck) {
        mgmt.send(buildAck(cmdId, r.ackStatus));
    }
    if (r.sendButtonToggle) {
        mgmt.send(buildButtonToggle(wifiManager.getEpochTime()));
    }
    if (r.sendHeartbeat) {
        mgmt.send(buildHeartbeat(makeHeartbeat(ctx, in)));
        buttonPressCount = 0;  // reset the parity counter after reporting
    }
    if (r.doRestart) {
        NVIC_SystemReset();
    }
    if (r.setLed >= 0) {
        digitalWrite(STATUS_LED_PIN, r.setLed ? HIGH : LOW);
    }
    if (r.delayMs > 0) {
        delay(r.delayMs);
    }

    unsigned long elapsed = millis() - loopStart;
    if (elapsed > loopMaxMs) loopMaxMs = elapsed;
}
