#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H

#include <Arduino.h>

// ========== State Machine Types ==========

enum State {
    BOOTING,
    CONNECTING_WIFI,
    IDLE,
    STARTING_SHOW,
    AUTO_DJ_ACTIVE,
    ENDING_SHOW,
    ERROR_STATE
};

/**
 * Persisted state carried across ticks.
 */
struct Context {
    State state;
    int radioShowID;
    int retryCount;
    unsigned long lastPollTime;
};

/**
 * Per-tick snapshot of sensor state, I/O results, and configuration.
 * The orchestrator fills this before calling tick().
 */
struct Inputs {
    // Sensor / network state
    bool relayStateChanged;
    bool autoDJActive;
    bool wifiConnected;
    unsigned long epochTime;
    unsigned long currentMillis;

    // I/O results (filled by orchestrator for the current state)
    int startShowResult;    // radioShowID or -1 on failure
    bool endShowResult;     // success?
    bool pollNewTrack;      // new track detected?
    bool pollLiveDJ;        // live DJ streaming?
    String artist;
    String title;
    String album;

    // Config constants (avoids #define dependency in pure code)
    unsigned long pollIntervalMs;
    int maxRetries;
    unsigned long retryBackoffMs;
};

/**
 * Output from tick(): updated context plus post-transition actions.
 */
struct TickResult {
    Context context;

    // Post-transition actions for the orchestrator
    bool addEntry;
    unsigned long addEntryHourMs;
    String addEntryArtist;
    String addEntryTitle;
    String addEntryAlbum;

    unsigned long delayMs;
};

/**
 * Pure state machine transition function. Takes current context and a snapshot
 * of inputs; returns updated context and any actions for the orchestrator to
 * execute. Has no side effects (no Serial, WiFi, GPIO, or HTTP).
 */
TickResult tick(const Context& ctx, const Inputs& inputs);

/**
 * Returns a human-readable name for the given state.
 */
const char* stateName(State s);

#endif
