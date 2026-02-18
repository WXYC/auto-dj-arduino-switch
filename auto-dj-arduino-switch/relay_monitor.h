#ifndef RELAY_MONITOR_H
#define RELAY_MONITOR_H

#include <Arduino.h>

/**
 * Monitors the mixing board AUX relay contact with software debouncing.
 *
 * The relay contact is wired between RELAY_PIN and GND. When the relay
 * closes (AUX off = auto DJ active), the pin reads LOW via INPUT_PULLUP.
 * When the relay opens (AUX on = DJ live), the pin reads HIGH.
 *
 * Call update() every loop iteration. Check stateChanged() for edge
 * detection and isAutoDJActive() for current debounced state.
 */
class RelayMonitor {
public:
    RelayMonitor(int relayPin, int ledPin, unsigned long debounceMs);
    void setUp();
    void update();
    bool isAutoDJActive() const;
    bool stateChanged() const;

private:
    int relayPin;
    int ledPin;
    unsigned long debounceMs;

    int debouncedState;
    int lastReading;
    unsigned long lastChangeTime;
    bool changed;
};

#endif
