# Auto DJ Arduino Switch

Arduino Giga R1 WiFi + Ethernet Shield Rev2 that bridges WXYC's auto DJ (AzuraCast) with the tubafrenzy flowsheet. When no live DJ is broadcasting, the Arduino detects this via a relay contact and writes currently-playing tracks to the flowsheet.

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
- **Architecture:** Pure-function state machine. `tick()` takes `Context` + `Inputs`, returns `TickResult` with updated context + actions. The `.ino` `loop()` is a thin I/O orchestrator.
- **Config:** `config.h` (pin assignments, timing, server endpoints), `secrets.h` (WiFi password, API key -- gitignored)
- **Libraries:** ArduinoHttpClient, ArduinoJson v7+

### Key files

| File | Role |
|------|------|
| `state_machine.h/.cpp` | Pure `tick()` function: all decision logic |
| `utils.h/.cpp` | `urlEncode`, `parseRadioShowID`, `currentHourMs` |
| `azuracast_client.h/.cpp` | AzuraCast Now Playing API client |
| `flowsheet_client.h/.cpp` | tubafrenzy flowsheet API client |
| `relay_monitor.h/.cpp` | Debounced relay input |
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

When adding new pure-logic functions, extract them into `utils.h` or `state_machine.h` so they can be tested on desktop. I/O-performing code stays in the `.ino` file and the `*_client` modules.

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
