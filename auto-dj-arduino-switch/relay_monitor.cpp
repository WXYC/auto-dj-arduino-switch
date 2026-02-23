#include "relay_monitor.h"

RelayMonitor::RelayMonitor(int relayPin, int ledPin, unsigned long debounceMs)
    : relayPin(relayPin)
    , ledPin(ledPin)
    , debounceMs(debounceMs)
    , debouncedState(HIGH)
    , lastReading(HIGH)
    , lastChangeTime(0)
    , changed(false)
    , ledState(LOW)
{
}

void RelayMonitor::setUp() {
    pinMode(relayPin, INPUT_PULLUP);
    pinMode(ledPin, OUTPUT);
    debouncedState = digitalRead(relayPin);
    lastReading = debouncedState;
    ledState = debouncedState == LOW ? HIGH : LOW;
    digitalWrite(ledPin, ledState);
}

void RelayMonitor::update() {
    update(millis(), digitalRead(relayPin));
    digitalWrite(ledPin, ledState);
}

void RelayMonitor::update(unsigned long currentMillis, int currentReading) {
    changed = false;

    if (currentReading != lastReading) {
        lastChangeTime = currentMillis;
    }

    if ((currentMillis - lastChangeTime) > debounceMs) {
        if (currentReading != debouncedState) {
            debouncedState = currentReading;
            changed = true;
            // LED on when auto DJ is active (relay closed = LOW)
            ledState = debouncedState == LOW ? HIGH : LOW;
        }
    }

    lastReading = currentReading;
}

bool RelayMonitor::isAutoDJActive() const {
    // Relay closed (pin LOW via pullup) = AUX off = auto DJ active
    return debouncedState == LOW;
}

bool RelayMonitor::stateChanged() const {
    return changed;
}

int RelayMonitor::getLedState() const {
    return ledState;
}
