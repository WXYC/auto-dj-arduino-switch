#ifndef MGMT_CLIENT_H
#define MGMT_CLIENT_H

#include <Arduino.h>
#include "state_machine.h"
#include "mgmt_protocol.h"

// I/O wrapper for the orchestrator management channel. All JSON assembly/parsing
// lives in mgmt_protocol (pure, desktop-tested); this class only does sockets:
// the WebSocket (Ethernet primary) and the HTTP poll fallback (WiFi). It is
// NOT compiled in CI -- it #include <ArduinoWebsockets.h>, which is unavailable
// on the desktop runner.
#include <ArduinoWebsockets.h>

class MgmtClient {
public:
    MgmtClient(const char* host, int port, const char* wsPath, const char* hbPath,
               const char* cmdPath, const char* authKey, bool useTls);

    /** Open the WebSocket and register callbacks. Returns true on success. */
    bool connectWs();

    /** Pump the WebSocket receive loop. Call every loop iteration. */
    void poll();

    bool channelUp() const { return connected; }

    /** Send a pre-assembled JSON frame over the WebSocket. */
    void send(const String& payload);

    /**
     * Take a command parsed since the last call. Returns true and fills action/id
     * if one is pending.
     */
    bool takeCommand(CommandAction* action, String* id);

    /** Take an orchestrator-reported active flag (from a button_toggle ack). */
    bool takeActiveResult(bool* active);

    /** HTTP fallback (WiFi): POST a heartbeat body. Returns true on 200. */
    bool httpHeartbeat(const String& body);

private:
    void onMessage(const String& raw);

    const char* host;
    int port;
    const char* wsPath;
    const char* hbPath;
    const char* cmdPath;
    const char* authKey;
    bool useTls;

    websockets::WebsocketsClient ws;
    bool connected = false;

    // Single-slot inboxes filled by onMessage(), drained by the .ino.
    bool pendingCommand = false;
    CommandAction pendingAction = CMD_NONE;
    String pendingId;
    bool pendingActiveResult = false;
    bool pendingActive = false;
};

#endif
