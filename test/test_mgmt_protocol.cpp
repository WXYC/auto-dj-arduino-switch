#include <gtest/gtest.h>
#include <ArduinoJson.h>
#include "mgmt_protocol.h"
#include "test_helpers.h"

static HeartbeatFields baseFields() {
    HeartbeatFields f;
    f.state = "CONNECTED";
    f.transport = "ethernet";
    f.uptime_s = 86402;
    f.wifi_rssi_null = true;
    f.wifi_rssi = 0;
    f.free_ram = 524288;
    f.firmware_version = "2.0.0";
    f.config_hash = "a3f2c8";
    f.loop_max_ms = 45;
    f.reconnect_count = 0;
    f.tracks_detected = 0;
    f.tracks_posted = 0;
    f.errors_since_boot = 2;
    f.button_press_count = 0;
    f.relay_auto_dj_active = true;
    return f;
}

TEST(MgmtProtocol, HeartbeatAssemblesAllFields) {
    String json = buildHeartbeat(baseFields());
    JsonDocument doc;
    ASSERT_FALSE(deserializeJson(doc, json));
    EXPECT_STREQ(doc["type"], "heartbeat");
    EXPECT_STREQ(doc["state"], "CONNECTED");
    EXPECT_STREQ(doc["transport"], "ethernet");
    EXPECT_EQ(doc["uptime_s"].as<unsigned long>(), 86402UL);
    EXPECT_TRUE(doc["wifi_rssi"].isNull());        // null on ethernet
    EXPECT_TRUE(doc["radio_show_id"].isNull());    // reporter model: no show
    EXPECT_EQ(doc["errors_since_boot"].as<int>(), 2);
    EXPECT_TRUE(doc["relay_auto_dj_active"].as<bool>());
}

TEST(MgmtProtocol, HeartbeatReflectsRelayAndWifiRssi) {
    HeartbeatFields f = baseFields();
    f.relay_auto_dj_active = false;
    f.transport = "wifi";
    f.wifi_rssi_null = false;
    f.wifi_rssi = -67;
    JsonDocument doc;
    ASSERT_FALSE(deserializeJson(doc, buildHeartbeat(f)));
    EXPECT_FALSE(doc["relay_auto_dj_active"].as<bool>());
    EXPECT_EQ(doc["wifi_rssi"].as<int>(), -67);
}

TEST(MgmtProtocol, ButtonToggleAssembly) {
    JsonDocument doc;
    ASSERT_FALSE(deserializeJson(doc, buildButtonToggle(1709852100UL)));
    EXPECT_STREQ(doc["type"], "button_toggle");
    EXPECT_EQ(doc["timestamp"].as<unsigned long>(), 1709852100UL);
    EXPECT_EQ(buttonToggleAckId(1709852100UL), String("btn_1709852100"));
}

TEST(MgmtProtocol, AckAssemblyMapsStatus) {
    JsonDocument ok;
    ASSERT_FALSE(deserializeJson(ok, buildAck("cmd_1", ACK_OK)));
    EXPECT_STREQ(ok["type"], "ack");
    EXPECT_STREQ(ok["id"], "cmd_1");
    EXPECT_STREQ(ok["status"], "ok");

    JsonDocument unknown;
    ASSERT_FALSE(deserializeJson(unknown, buildAck("x", ACK_UNKNOWN_COMMAND)));
    EXPECT_STREQ(unknown["status"], "unknown_command");
}

TEST(MgmtProtocol, ParseCommandEachAction) {
    EXPECT_EQ(parseCommand("{\"type\":\"command\",\"id\":\"a\",\"action\":\"pause\"}").action, CMD_PAUSE);
    EXPECT_EQ(parseCommand("{\"type\":\"command\",\"id\":\"a\",\"action\":\"resume\"}").action, CMD_RESUME);
    EXPECT_EQ(parseCommand("{\"type\":\"command\",\"id\":\"a\",\"action\":\"ping\"}").action, CMD_PING);
    EXPECT_EQ(parseCommand("{\"type\":\"command\",\"id\":\"a\",\"action\":\"restart\"}").action, CMD_RESTART);
    EXPECT_EQ(parseCommand("{\"type\":\"command\",\"id\":\"a\",\"action\":\"end_show\"}").action, CMD_END_SHOW);
    EXPECT_EQ(parseCommand("{\"type\":\"command\",\"id\":\"a\",\"action\":\"bogus\"}").action, CMD_UNKNOWN);
}

TEST(MgmtProtocol, ParseCommandSetConfigExposesKeyValue) {
    ParsedCommand pc = parseCommand(
        "{\"type\":\"command\",\"id\":\"c1\",\"action\":\"set_config\",\"key\":\"poll_interval_ms\",\"value\":\"30000\"}");
    EXPECT_TRUE(pc.valid);
    EXPECT_EQ(pc.action, CMD_SET_CONFIG);
    EXPECT_EQ(pc.id, String("c1"));
    EXPECT_EQ(pc.key, String("poll_interval_ms"));
    EXPECT_EQ(pc.value, String("30000"));
}

TEST(MgmtProtocol, ParseCommandRejectsMalformedAndNonCommand) {
    EXPECT_FALSE(parseCommand("not json").valid);
    EXPECT_FALSE(parseCommand("{\"type\":\"ack\",\"id\":\"x\"}").valid);
}

TEST(MgmtProtocol, ParseAckResultActive) {
    ParsedAckResult r = parseAckResult("{\"type\":\"ack\",\"id\":\"btn_1\",\"status\":\"ok\",\"result\":{\"active\":true}}");
    EXPECT_TRUE(r.hasActive);
    EXPECT_TRUE(r.active);

    ParsedAckResult inactive = parseAckResult("{\"type\":\"ack\",\"id\":\"btn_2\",\"status\":\"ok\",\"result\":{\"active\":false}}");
    EXPECT_TRUE(inactive.hasActive);
    EXPECT_FALSE(inactive.active);
}

TEST(MgmtProtocol, ParseAckResultAbsentResult) {
    ParsedAckResult r = parseAckResult("{\"type\":\"ack\",\"id\":\"cmd_1\",\"status\":\"ok\"}");
    EXPECT_FALSE(r.hasActive);
}
