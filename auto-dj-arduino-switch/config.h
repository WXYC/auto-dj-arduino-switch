#ifndef CONFIG_H
#define CONFIG_H

// ========== Pin Assignments ==========
#define RELAY_PIN 2        // Mixing board AUX relay contact (INPUT_PULLUP)
#define STATUS_LED_PIN 3   // External status LED

// ========== Timing (milliseconds) ==========
#define DEBOUNCE_MS 50
#define POLL_INTERVAL_MS 20000         // 20s AzuraCast poll
#define WIFI_RETRY_INTERVAL_MS 5000    // 5s WiFi reconnect delay
#define HTTP_RESPONSE_TIMEOUT_MS 10000 // 10s HTTP timeout
#define NTP_SYNC_INTERVAL_MS 3600000UL // Re-sync NTP every hour
#define MAX_RETRIES 3
#define RETRY_BACKOFF_MS 2000          // Base backoff between retries

// ========== AzuraCast ==========
// Use the static JSON endpoint (Nginx-cached, lower server load)
#define AZURACAST_HOST "remote.wxyc.org"
#define AZURACAST_PORT 443
#define AZURACAST_PATH "/api/nowplaying_static/main.json"

// ========== tubafrenzy ==========
#define TUBAFRENZY_HOST "www.wxyc.info"
#define TUBAFRENZY_PORT 443
#define TUBAFRENZY_PATH_START_SHOW "/playlists/startRadioShow"
#define TUBAFRENZY_PATH_ADD_ENTRY  "/playlists/flowsheetEntryAdd"
#define TUBAFRENZY_PATH_END_SHOW   "/playlists/finishRadioShow"

// ========== Auto DJ Identity ==========
// These are written directly to the FLOWSHEET_RADIO_SHOW_PROD table --
// there is no separate DJ table, so no migration needed.
#define AUTO_DJ_ID "0"
#define AUTO_DJ_NAME "Auto DJ"
#define AUTO_DJ_HANDLE "AutoDJ"
#define AUTO_DJ_SHOW_NAME "Auto DJ"

// ========== NTP ==========
#define NTP_SERVER "pool.ntp.org"
#define UTC_OFFSET_SECONDS -18000 // Eastern Time (UTC-5)

#endif
