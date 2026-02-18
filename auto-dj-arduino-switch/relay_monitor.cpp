#include "relay_monitor.h"

RelayMonitor::RelayMonitor(int relayPin, int ledPin, unsigned long debounceMs)
    : relayPin(relayPin)
    , ledPin(ledPin)
    , debounceMs(debounceMs)
    , debouncedState(HIGH)
    , lastReading(HIGH)
    , lastChangeTime(0)
    , changed(false)
{
}

void RelayMonitor::setUp() {
    pinMode(relayPin, INPUT_PULLUP);
    pinMode(ledPin, OUTPUT);
    debouncedState = digitalRead(relayPin);
    lastReading = debouncedState;
    digitalWrite(ledPin, debouncedState == LOW ? HIGH : LOW);
}

void RelayMonitor::update() {
    changed = false;
    int reading = digitalRead(relayPin);

    if (reading != lastReading) {
        lastChangeTime = millis();
    }

    if ((millis() - lastChangeTime) > debounceMs) {
        if (reading != debouncedState) {
            debouncedState = reading;
            changed = true;
            // LED on when auto DJ is active (relay closed = LOW)
            digitalWrite(ledPin, debouncedState == LOW ? HIGH : LOW);
        }
    }

    lastReading = reading;
}

bool RelayMonitor::isAutoDJActive() const {
    // Relay closed (pin LOW via pullup) = AUX off = auto DJ active
    return debouncedState == LOW;
}

bool RelayMonitor::stateChanged() const {
    return changed;
}
