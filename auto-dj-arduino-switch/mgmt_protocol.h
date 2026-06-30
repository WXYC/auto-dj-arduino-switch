#ifndef MGMT_PROTOCOL_H
#define MGMT_PROTOCOL_H

#include <Arduino.h>
#include "state_machine.h"  // CommandAction, AckStatus (pure enums)

// ========== Management-channel JSON (pure) ==========
//
// Byte-in / byte-out assembly and parsing of the orchestrator <-> Arduino
// messages (networking-spec §3.6.2). No sockets: mgmt_client owns I/O and
// delegates all JSON here, so the protocol is fully desktop-testable.

/** Fields for an outbound heartbeat. */
struct HeartbeatFields {
    const char* state;            // state name (e.g. "CONNECTED")
    const char* transport;        // "ethernet" | "wifi"
    unsigned long uptime_s;
    bool wifi_rssi_null;          // true on Ethernet (rssi reported as null)
    long wifi_rssi;
    unsigned long free_ram;
    const char* firmware_version;
    const char* config_hash;
    unsigned long loop_max_ms;
    unsigned int reconnect_count;
    unsigned int tracks_detected;
    unsigned int tracks_posted;
    unsigned int errors_since_boot;
    unsigned int button_press_count;
    bool relay_auto_dj_active;
};

/** A parsed inbound command. */
struct ParsedCommand {
    bool valid;
    CommandAction action;
    String id;
    String key;
    String value;
};

/** A parsed ack carrying an optional result.active. */
struct ParsedAckResult {
    bool hasActive;
    bool active;
};

/** Assemble a heartbeat frame. radio_show_id and last_track are null (reporter model). */
String buildHeartbeat(const HeartbeatFields& f);

/** Assemble a button_toggle frame. */
String buildButtonToggle(unsigned long timestamp);

/** Derive the ack id for a button_toggle press ("btn_<timestamp>"). */
String buttonToggleAckId(unsigned long timestamp);

/** Assemble an ack frame. */
String buildAck(const String& id, AckStatus status);

/** Parse an inbound command frame. Sets valid=false on malformed/unknown JSON. */
ParsedCommand parseCommand(const String& json);

/** Parse an inbound ack carrying result.active (orchestrator -> Arduino). */
ParsedAckResult parseAckResult(const String& json);

#endif
