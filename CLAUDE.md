# Auto DJ Arduino Switch

Arduino Giga R1 WiFi + Ethernet Shield Rev2 that **reports** the mixing board's AUX relay state and a manual toggle button to the [auto-dj-orchestrator](https://github.com/WXYC/auto-dj-orchestrator) over a WebSocket management channel (HTTP fallback over WiFi). It is a "dumb" reporter: the orchestrator owns all activation logic, subscribes to AzuraCast, and writes the flowsheet. This board only reports inputs and drives the status LED.

## Repository Layout

```
auto-dj-arduino-switch/   Arduino sketch (firmware)
test/                     Desktop tests (GoogleTest + Arduino shim)
enclosure/                3D-printed enclosure (OpenSCAD + Python tools)
docs/                     Specifications and reference docs
.github/workflows/        CI (desktop tests on push/PR)
```

## Firmware (`auto-dj-arduino-switch/`)

- **Language:** C++ (Arduino, C++14 for tests)
- **Board:** Arduino Giga R1 WiFi (Mbed OS GIGA board package)
- **Architecture:** Pure-function state machine (connectivity + LED, no show lifecycle). `tick()` takes `Context` + `Inputs`, returns `TickResult` with updated context + actions. The `.ino` `loop()` is a thin I/O orchestrator.
- **Config:** `config.h` (pin assignments, timing, orchestrator endpoints), `secrets.h` (WiFi password, `AUTO_DJ_KEY` -- gitignored)
- **Libraries:** ArduinoWebsockets, ArduinoHttpClient, ArduinoJson v7+

### Key files

| File | Role |
|------|------|
| `state_machine.h/.cpp` | Pure `tick()`: connectivity transitions, command handling, LED policy |
| `relay_monitor.h/.cpp` | Debounced relay input |
| `button_monitor.h/.cpp` | Debounced manual toggle button input (press-edge) |
| `mgmt_protocol.h/.cpp` | **Pure** management-channel JSON: heartbeat/button_toggle/ack assembly, command/ack-result parse (desktop-tested) |
| `mgmt_client.h/.cpp` | I/O wrapper: WebSocket (Ethernet) + HTTP poll (WiFi). Delegates all JSON to `mgmt_protocol`. **Not CI-compiled** (pulls in `ArduinoWebsockets.h`) |
| `wifi_manager.h/.cpp` | WiFi connection management |
| `config.h` | All compile-time constants |

## Testing

Desktop tests using GoogleTest with an Arduino `String` shim (no hardware required):

```bash
cmake -B test/build test/
cmake --build test/build
cd test/build && ctest --output-on-failure
```

CI runs on every push/PR to `main` via `.github/workflows/test.yml`.

When adding new pure logic, put it in `state_machine.h` or `mgmt_protocol.h` so it can be tested on desktop. I/O-performing code stays in the `.ino` and `mgmt_client`. **`mgmt_client.cpp` is deliberately excluded from the CMake build** (it `#include <ArduinoWebsockets.h>`, unavailable in CI); keep all assembly/parsing in `mgmt_protocol` so the WS library's absence costs no coverage, and never include `mgmt_client.h` from `state_machine.h` / `button_monitor.h`.

## Enclosure (`enclosure/`)

OpenSCAD parametric models + Python analysis tools. All dimensions in `config.scad`.

```bash
cd enclosure
make stls             # Render STLs (base ~90s, others fast)
make cross-sections   # Python-based cross-section SVGs
make slices           # Test slice STLs for fit checks
make verify           # Validate cutout positions
make analyze          # Bounding box analysis of original STLs
```

See [enclosure/README.md](enclosure/README.md) for full build/assembly instructions.

### Enclosure conventions

- All measurements in millimeters
- Dimensions derived from `analyze_stl.py` output and EAGLE BRD data
- Python scripts have no external dependencies (stdlib only)
- Cross-sections use Python STL slicing (not OpenSCAD `projection`, which fails on complex meshes)

## Development Notes

- The state machine is the core abstraction. Keep `tick()` pure -- no I/O, no global state, no `delay()`. This makes it fully testable on desktop.
- `secrets.h` is gitignored. Copy from `secrets.h.example`.
- The Giga R1's WiFi reconnection blocks for ~36s (firmware limitation). The state machine cannot run during this window.
- The enclosure's `output/` directory is gitignored. Regenerate with `make -C enclosure all`.
- The physical toggle button sends a `button_toggle` message to the orchestrator via the management channel. All activation logic lives in the orchestrator -- the button does not directly control the Arduino's state machine. See [docs/plan-button-and-virtual-switch.md](docs/plan-button-and-virtual-switch.md) for the virtual switch API spec and dj-site UI design.
