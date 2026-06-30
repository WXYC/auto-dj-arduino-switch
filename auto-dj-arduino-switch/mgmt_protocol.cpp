#include "mgmt_protocol.h"
#include <ArduinoJson.h>

static CommandAction actionFromString(const String& s) {
    if (s == "set_config") return CMD_SET_CONFIG;
    if (s == "pause") return CMD_PAUSE;
    if (s == "resume") return CMD_RESUME;
    if (s == "end_show") return CMD_END_SHOW;
    if (s == "restart") return CMD_RESTART;
    if (s == "ping") return CMD_PING;
    return CMD_UNKNOWN;
}

String buildHeartbeat(const HeartbeatFields& f) {
    JsonDocument doc;
    doc["type"] = "heartbeat";
    doc["state"] = f.state;
    doc["transport"] = f.transport;
    doc["uptime_s"] = f.uptime_s;
    if (f.wifi_rssi_null) {
        doc["wifi_rssi"] = nullptr;
    } else {
        doc["wifi_rssi"] = f.wifi_rssi;
    }
    doc["free_ram"] = f.free_ram;
    doc["radio_show_id"] = nullptr;  // reporter model: no show
    doc["last_error"] = nullptr;
    doc["firmware_version"] = f.firmware_version;
    doc["config_hash"] = f.config_hash;
    doc["loop_max_ms"] = f.loop_max_ms;
    doc["reconnect_count"] = f.reconnect_count;
    doc["tracks_detected"] = f.tracks_detected;
    doc["tracks_posted"] = f.tracks_posted;
    doc["errors_since_boot"] = f.errors_since_boot;
    doc["button_press_count"] = f.button_press_count;
    doc["relay_auto_dj_active"] = f.relay_auto_dj_active;

    String out;
    serializeJson(doc, out);
    return out;
}

String buildButtonToggle(unsigned long timestamp) {
    JsonDocument doc;
    doc["type"] = "button_toggle";
    doc["timestamp"] = timestamp;
    String out;
    serializeJson(doc, out);
    return out;
}

String buttonToggleAckId(unsigned long timestamp) {
    return String("btn_") + String(timestamp);
}

String buildAck(const String& id, AckStatus status) {
    JsonDocument doc;
    doc["type"] = "ack";
    doc["id"] = id;
    doc["status"] = status == ACK_OK ? "ok" : status == ACK_ERROR ? "error" : "unknown_command";
    String out;
    serializeJson(doc, out);
    return out;
}

ParsedCommand parseCommand(const String& json) {
    ParsedCommand pc;
    pc.valid = false;
    pc.action = CMD_UNKNOWN;

    JsonDocument doc;
    if (deserializeJson(doc, json)) return pc;

    String type = doc["type"] | "";
    if (type != "command") return pc;

    pc.id = doc["id"] | "";
    pc.key = doc["key"] | "";
    pc.value = doc["value"] | "";
    pc.action = actionFromString(doc["action"] | "");
    pc.valid = true;
    return pc;
}

ParsedAckResult parseAckResult(const String& json) {
    ParsedAckResult r;
    r.hasActive = false;
    r.active = false;

    JsonDocument doc;
    if (deserializeJson(doc, json)) return r;

    String type = doc["type"] | "";
    if (type != "ack") return r;

    JsonVariant result = doc["result"];
    if (!result.isNull() && result["active"].is<bool>()) {
        r.hasActive = true;
        r.active = result["active"].as<bool>();
    }
    return r;
}
