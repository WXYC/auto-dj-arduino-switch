# Wiring Guide

## Hardware

- **Board:** Arduino Giga R1 WiFi
- **Relay:** Dry contact from mixing board AUX channel
- **Button:** Industrial 22mm mushroom head, momentary (spring return), 1NO+1NC, in control station enclosure
- **LED:** Standard LED + 220 ohm resistor (optional status indicator)

## Pin Assignments

| Pin | Function | Mode | Wiring |
|-----|----------|------|--------|
| D2 | Relay contact input | `INPUT_PULLUP` | Relay terminal -> D2, other terminal -> GND |
| D3 | Status LED | `OUTPUT` | D3 -> 220 ohm -> LED anode, cathode -> GND |
| D5 | Manual toggle button | `INPUT_PULLUP` | Button NO terminal -> D5, common terminal -> GND |
| GND | Common ground | -- | Shared with relay, button, and LED |
| LED_BUILTIN | Heartbeat | `OUTPUT` | Onboard LED (blinks every second) |

## Wiring Diagram

```
Mixing Board AUX Relay (dry contact)
    Terminal A ──────── D2 (INPUT_PULLUP)
    Terminal B ──────── GND

Manual Toggle Button (22mm mushroom head, momentary)
    NO terminal ──────── D5 (INPUT_PULLUP)
    Common terminal ──── GND

Status LED
    D3 ─── 220 ohm ─── LED (+) ─── LED (-) ─── GND
```

## How It Works

The mixing board has an AUX relay contact that reflects whether the AUX channel is enabled:

- **Relay open** (D2 reads HIGH via internal pullup): AUX is enabled, a live DJ is broadcasting.
- **Relay closed** (D2 reads LOW, pulled to GND): AUX is disabled, auto DJ is active.

The status LED mirrors this state:
- **LED on:** Auto DJ is active (relay closed).
- **LED off:** DJ is live (relay open).

The built-in LED blinks once per second as a heartbeat indicator.

## Manual Toggle Button

A 22mm industrial mushroom head button (momentary, spring return) in a control station enclosure box provides a physical toggle for the auto-DJ system. Pressing the button sends a toggle command to the orchestrator via the management channel. The orchestrator decides whether to activate or deactivate based on its current state.

The button uses the NO (normally open) contact. At rest, D5 reads HIGH (internal pullup). When pressed, the contact closes and D5 reads LOW (pulled to GND). The `ButtonMonitor` class detects this transition using the same 50ms debounce pattern as the relay.

The button does NOT directly control the Arduino's state machine. All activation logic lives in the orchestrator. Before the orchestrator is deployed, the button is wired and debounced but has no effect -- the Arduino continues to operate in relay-only mode.

**Pin choice:** D5 is used because D4 is the Ethernet Shield Rev2's SD card chip select (the shield's PCB connects D4 to the SD card slot even when unused). D10-D13 are reserved for the Ethernet Shield SPI bus.

## Debouncing

Both the relay and the button use 50ms software debounce. The pin state must remain stable for 50ms before a state change is registered. The relay debounce is in `relay_monitor.cpp`; the button debounce follows the same pattern in `button_monitor.cpp`.
