#ifndef BUTTON_MONITOR_H
#define BUTTON_MONITOR_H

#include <Arduino.h>

/**
 * Monitors the manual toggle button (22mm mushroom head, momentary) with
 * software debouncing. Wired NO contact: at rest the pin reads HIGH via
 * INPUT_PULLUP; pressing pulls it LOW.
 *
 * Call update() every loop iteration. pressed() is true for exactly one update
 * after a debounced press (HIGH -> LOW edge). The button does not drive the
 * state machine directly -- a press emits a button_toggle to the orchestrator,
 * which decides whether to activate or deactivate.
 */
class ButtonMonitor {
public:
    ButtonMonitor(int buttonPin, unsigned long debounceMs);
    void setUp();

    /** Hardware wrapper: reads the button pin and runs the debounce. */
    void update();

    /**
     * Parameterized update for desktop testing. All debounce logic, no hardware.
     */
    void update(unsigned long currentMillis, int currentReading);

    /** True for one update after a debounced press (HIGH -> LOW commit). */
    bool pressed() const;

    /** Current debounced level (true = button held down / LOW). */
    bool isDown() const;

private:
    int buttonPin;
    unsigned long debounceMs;

    int debouncedState;   // HIGH (released) at rest
    int lastReading;
    unsigned long lastChangeTime;
    bool pressedEdge;
};

#endif
