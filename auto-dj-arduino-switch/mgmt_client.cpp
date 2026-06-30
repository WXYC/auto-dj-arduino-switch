#include "mgmt_client.h"
#include <ArduinoHttpClient.h>
#include <WiFi.h>
#include <WiFiSSLClient.h>

using namespace websockets;

MgmtClient::MgmtClient(const char* host, int port, const char* wsPath, const char* hbPath,
                       const char* cmdPath, const char* authKey, bool useTls)
    : host(host), port(port), wsPath(wsPath), hbPath(hbPath), cmdPath(cmdPath),
      authKey(authKey), useTls(useTls) {}

bool MgmtClient::connectWs() {
    ws.addHeader("X-Auto-DJ-Key", authKey);
    ws.onMessage([this](WebsocketsMessage msg) { onMessage(msg.data()); });
    ws.onEvent([this](WebsocketsEvent event, String) {
        if (event == WebsocketsEvent::ConnectionClosed) {
            connected = false;
        }
    });

    String scheme = useTls ? "wss://" : "ws://";
    String url = scheme + String(host) + ":" + String(port) + String(wsPath);
    connected = ws.connect(url);
    return connected;
}

void MgmtClient::poll() {
    if (connected) {
        ws.poll();
    }
}

void MgmtClient::send(const String& payload) {
    if (connected) {
        ws.send(payload);
    }
}

void MgmtClient::onMessage(const String& raw) {
    ParsedCommand cmd = parseCommand(raw);
    if (cmd.valid) {
        pendingCommand = true;
        pendingAction = cmd.action;
        pendingId = cmd.id;
        return;
    }
    ParsedAckResult ack = parseAckResult(raw);
    if (ack.hasActive) {
        pendingActiveResult = true;
        pendingActive = ack.active;
    }
}

bool MgmtClient::takeCommand(CommandAction* action, String* id) {
    if (!pendingCommand) return false;
    *action = pendingAction;
    *id = pendingId;
    pendingCommand = false;
    return true;
}

bool MgmtClient::takeActiveResult(bool* active) {
    if (!pendingActiveResult) return false;
    *active = pendingActive;
    pendingActiveResult = false;
    return true;
}

bool MgmtClient::httpHeartbeat(const String& body) {
    // Per-call client (Giga R1 "destroy after each call" workaround).
    WiFiSSLClient ssl;
    WiFiClient plain;
    Client& transport = useTls ? (Client&)ssl : (Client&)plain;
    HttpClient http(transport, host, port);
    http.beginRequest();
    http.post(hbPath);
    http.sendHeader("Content-Type", "application/json");
    http.sendHeader("X-Auto-DJ-Key", authKey);
    http.sendHeader("Content-Length", body.length());
    http.beginBody();
    http.print(body);
    http.endRequest();
    int status = http.responseStatusCode();
    http.stop();
    return status == 200;
}
