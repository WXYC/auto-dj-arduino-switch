#ifndef CONFIG_H
#define CONFIG_H

// ========== Pin Assignments ==========
#define RELAY_PIN 2        // Mixing board AUX relay contact (INPUT_PULLUP)
#define STATUS_LED_PIN 3   // External status LED (on = auto-DJ active)
#define BUTTON_PIN 5       // Manual toggle button, INPUT_PULLUP, NO contact

// ========== Timing (milliseconds) ==========
#define DEBOUNCE_MS 50
#define WS_HEARTBEAT_MS 30000          // Management-channel heartbeat (Ethernet/WS)
#define HTTP_POLL_INTERVAL_MS 60000    // Management-channel poll (WiFi fallback)
#define WIFI_RETRY_INTERVAL_MS 5000    // 5s WiFi reconnect delay
#define MAX_RETRIES 3
#define RETRY_BACKOFF_MS 2000          // Base backoff between transport retries

// ========== Orchestrator management channel ==========
// In local/staging the orchestrator is plain ws:// on the host's LAN IP; in
// production it is wss:// behind a hostname. Set ORCHESTRATOR_USE_TLS to 1 for
// production.
#define ORCHESTRATOR_HOST "192.168.1.50"   // host LAN IP (local/staging) or hostname (prod)
#define ORCHESTRATOR_PORT 8090
#define ORCHESTRATOR_WS_PATH "/api/auto-dj/ws"
#define ORCHESTRATOR_HB_PATH "/api/auto-dj/heartbeat"
#define ORCHESTRATOR_CMD_PATH "/api/auto-dj/commands"
#define ORCHESTRATOR_USE_TLS 0             // 0 = ws:// (local/staging), 1 = wss:// (prod)

// ========== Firmware ==========
#define FIRMWARE_VERSION "2.0.0"

// ========== NTP ==========
// Still needed for the button_toggle timestamp and heartbeat uptime telemetry.
#define NTP_SERVER "pool.ntp.org"
#define UTC_OFFSET_SECONDS -18000 // Eastern Time (UTC-5)

#endif
