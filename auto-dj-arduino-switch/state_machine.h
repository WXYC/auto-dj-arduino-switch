#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H

#include <Arduino.h>

// ========== Reporter state machine ==========
//
// The Arduino is a relay/button reporter: it no longer starts/ends shows,
// polls AzuraCast, or writes flowsheets. All activation logic lives in the
// auto-dj-orchestrator. The state machine tracks connectivity and drives the
// status LED; tick() stays pure (no Serial / sockets / delay).

enum State {
    BOOTING,      // pre-transport
    CONNECTING,   // bringing up a transport (Ethernet WS or WiFi HTTP)
    CONNECTED,    // a transport is up and the management channel is usable
    ERROR_STATE   // transport setup failed repeatedly; back off and retry
};

enum Transport {
    TRANSPORT_NONE,
    TRANSPORT_ETHERNET,
    TRANSPORT_WIFI
};

enum CommandAction {
    CMD_NONE,
    CMD_SET_CONFIG,
    CMD_PAUSE,
    CMD_RESUME,
    CMD_END_SHOW,
    CMD_RESTART,
    CMD_PING,
    CMD_UNKNOWN
};

enum AckStatus {
    ACK_OK,
    ACK_ERROR,
    ACK_UNKNOWN_COMMAND
};

/** Persisted state carried across ticks. */
struct Context {
    State state;
    bool relayAutoDJActive;   // last debounced relay level (true = relay reports auto-DJ-active)
    bool orchestratorActive;  // last orchestrator-reported active flag (for the LED)
    Transport transport;
    bool channelUp;           // management channel connected + authed
    int retryCount;           // transport (re)connect attempts, for ERROR backoff
    unsigned long lastHeartbeatMs;
};

/** Per-tick snapshot filled by the .ino orchestrator before calling tick(). */
struct Inputs {
    // Inputs from the debounced monitors
    bool relayChanged;        // relay debounced edge this tick
    bool relayAutoDJActive;   // current debounced relay level
    bool buttonPressed;       // button debounced press edge this tick

    // Transport / channel facts
    bool ethernetLinkUp;
    bool wifiConnected;
    bool channelUp;           // management channel currently connected + authed

    // Inbound-message facts (parsed by mgmt_protocol, passed in flat)
    bool gotCommand;
    CommandAction commandAction;
    bool gotActiveResult;     // an ack/command carried result.active
    bool activeResult;

    // Timing / config
    unsigned long currentMillis;
    unsigned long heartbeatIntervalMs;
    unsigned long retryBackoffMs;
    int maxRetries;
};

/** Output from tick(): updated context plus actions for the orchestrator. */
struct TickResult {
    Context context;

    bool sendHeartbeat;
    bool sendButtonToggle;
    bool sendAck;
    AckStatus ackStatus;
    bool doRestart;
    int setLed;               // -1 = leave, 0 = off, 1 = on

    unsigned long delayMs;    // ERROR backoff only
};

/**
 * Pure transition function. No I/O, no global state, no delay(). All sends are
 * requests in TickResult; the .ino performs them.
 */
TickResult tick(const Context& ctx, const Inputs& inputs);

/** Human-readable state name. */
const char* stateName(State s);

#endif
