# Auto DJ Arduino Switch

An Arduino Giga R1 WiFi sketch that bridges the gap between WXYC's auto DJ system ([AzuraCast](https://remote.wxyc.org)) and the station's flowsheet ([tubafrenzy](https://www.wxyc.info)). When no live DJ is broadcasting, the Arduino detects this via a relay contact on the mixing board, polls AzuraCast for currently-playing track data, and writes entries to the tubafrenzy flowsheet.

## Why?

When auto DJ is playing and no DJ is logged into the flowsheet, those tracks are not recorded in WXYC's playback history. This device fills that gap automatically.

## Architecture

```
Mixing Board           Arduino Giga R1 WiFi          AzuraCast
AUX Relay    ------>   Detects auto DJ active  <----> remote.wxyc.org
(dry contact)          via relay on D2                 (now playing API)
                            |
                            v
                       tubafrenzy (wxyc.info)
                       Flowsheet API
                       (start show, add entries, end show)
```

## State Machine

```
BOOTING -> CONNECTING_WIFI -> IDLE <-> STARTING_SHOW -> AUTO_DJ_ACTIVE -> ENDING_SHOW -> IDLE
                                                              |
                                                              v
                                                    (polls AzuraCast every 20s,
                                                     adds flowsheet entries)
```

- **IDLE:** Relay open (DJ is live). Waiting for auto DJ activation.
- **STARTING_SHOW:** Relay closed. Creating a new show on tubafrenzy.
- **AUTO_DJ_ACTIVE:** Polling AzuraCast, writing flowsheet entries. Server handles hourly breakpoints via `autoBreakpoint=true`.
- **ENDING_SHOW:** Relay opened. Signing off the show on tubafrenzy.

## Hardware

- Arduino Giga R1 WiFi
- Dry contact relay from mixing board AUX channel
- 220 ohm resistor + LED (optional status indicator)

See [docs/wiring.md](docs/wiring.md) for wiring details.

## Required Libraries

Install via Arduino Library Manager:

| Library | Version | Purpose |
|---------|---------|---------|
| ArduinoHttpClient | latest | HTTP GET/POST requests |
| ArduinoJson | v7+ | JSON parsing with filter documents |

The WiFi and WiFiSSLClient libraries are built into the Arduino Mbed OS GIGA board package.

## Setup

### 1. Install board support

In Arduino IDE, install **Arduino Mbed OS GIGA Boards** via Boards Manager.

### 2. Register the Arduino's MAC address

The Arduino connects to UNC-PSK WiFi. Register its MAC address at [UNC ITS](https://unc.edu/mydevices). To find the MAC address, upload a sketch that prints `WiFi.macAddress()` to Serial, or check the Serial output at boot (this sketch prints it during WiFi setup).

### 3. Configure secrets

```bash
cp auto-dj-arduino-switch/secrets.h.example auto-dj-arduino-switch/secrets.h
```

Edit `secrets.h` with:
- UNC-PSK WiFi password
- Auto DJ API key (must match the `AUTO_DJ_API_KEY` env var on the tubafrenzy server)

### 4. Server-side configuration

Set the `AUTO_DJ_API_KEY` environment variable on the tubafrenzy server (wxyc.info). The Arduino authenticates via the `X-Auto-DJ-Key` HTTP header, which is checked by `XYCCatalogServlet.validateControlRoomAccess()`.

### 5. Upload

Open `auto-dj-arduino-switch/auto-dj-arduino-switch.ino` in Arduino IDE, select the Giga R1 WiFi board, and upload.

### 6. Wire the relay

Connect the mixing board's AUX relay contact to pin D2 and GND. See [docs/wiring.md](docs/wiring.md).

## Configuration

Edit `config.h` to change:

- Pin assignments
- Polling interval (default 20s)
- Server hostnames and ports
- Auto DJ identity (DJ name, handle)
- NTP server and timezone offset

## Serial Monitor

The sketch logs state transitions, track detections, and API calls to Serial at 115200 baud. Connect via Arduino IDE Serial Monitor for debugging.

## Maintenance

### Annual UNC-PSK password change

The UNC-PSK password changes yearly. Update `secrets.h` and re-upload the sketch. The Arduino's MAC address may also need re-registration.

### API key rotation

To rotate the API key, update both:
1. `secrets.h` on the Arduino
2. `AUTO_DJ_API_KEY` env var on the tubafrenzy server

## Known Limitations

- **WiFi reconnection blocks for ~36 seconds** (known Giga R1 firmware limitation). During this time, the state machine is frozen. Track changes during a WiFi outage are not logged retroactively.
- **No watchdog timer** yet. Long-running reliability depends on the Giga R1's stability. A hardware watchdog could be added for 24/7 operation.
- **NTP dependency:** If NTP time sync fails, the show cannot start (the `startingHour` and `workingHour` parameters require epoch milliseconds).
