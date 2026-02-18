# Wiring Guide

## Hardware

- **Board:** Arduino Giga R1 WiFi
- **Relay:** Dry contact from mixing board AUX channel
- **LED:** Standard LED + 220 ohm resistor (optional status indicator)

## Pin Assignments

| Pin | Function | Mode | Wiring |
|-----|----------|------|--------|
| D2 | Relay contact input | `INPUT_PULLUP` | Relay terminal -> D2, other terminal -> GND |
| GND | Common ground | -- | Shared with relay contact |
| D3 | Status LED | `OUTPUT` | D3 -> 220 ohm -> LED anode, cathode -> GND |
| LED_BUILTIN | Heartbeat | `OUTPUT` | Onboard LED (blinks every second) |

## Wiring Diagram

```
Mixing Board AUX Relay (dry contact)
    Terminal A ──────── D2 (INPUT_PULLUP)
    Terminal B ──────── GND

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

## Debouncing

Relay contacts bounce for up to 50ms during transitions. The software debounce
in `relay_monitor.cpp` requires the pin state to remain stable for 50ms before
registering a state change.
