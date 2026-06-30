#include "button_monitor.h"

ButtonMonitor::ButtonMonitor(int buttonPin, unsigned long debounceMs)
    : buttonPin(buttonPin)
    , debounceMs(debounceMs)
    , debouncedState(HIGH)
    , lastReading(HIGH)
    , lastChangeTime(0)
    , pressedEdge(false)
{
}

void ButtonMonitor::setUp() {
    pinMode(buttonPin, INPUT_PULLUP);
    debouncedState = digitalRead(buttonPin);
    lastReading = debouncedState;
}

void ButtonMonitor::update() {
    update(millis(), digitalRead(buttonPin));
}

void ButtonMonitor::update(unsigned long currentMillis, int currentReading) {
    pressedEdge = false;

    if (currentReading != lastReading) {
        lastChangeTime = currentMillis;
    }

    if ((currentMillis - lastChangeTime) > debounceMs) {
        if (currentReading != debouncedState) {
            debouncedState = currentReading;
            // Press edge: a debounced transition to LOW (button pressed).
            if (debouncedState == LOW) {
                pressedEdge = true;
            }
        }
    }

    lastReading = currentReading;
}

bool ButtonMonitor::pressed() const {
    return pressedEdge;
}

bool ButtonMonitor::isDown() const {
    return debouncedState == LOW;
}
