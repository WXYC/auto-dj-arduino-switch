# Enclosure: Arduino GIGA R1 WiFi + Ethernet Shield Rev2

Modified 3D-printed enclosure for the auto DJ Arduino switch. Based on the [GIGA Display Shield case](https://www.printables.com/model/605051) (Printables model 605051), adapted to house an Ethernet Shield Rev2 instead of the Display Shield.

## Design Overview

Three-piece stacking enclosure:

1. **Base** -- main case body with GIGA R1 standoffs. Modified with RJ45 and terminal cutouts on the right wall, plus an internal support post.
2. **Shield shelf** -- internal platform that stacks inside the base. Modified with matching port cutouts.
3. **Lid** -- top cover. Display window filled solid for dust resistance.

Additionally:

4. **Terminal bracket** -- new part designed from scratch. Internal mount for the Euroblock screw terminal receptacle with strain relief.

## Board Orientation

The GIGA R1 WiFi (101.6 x 53.34 mm, Mega form factor) mounts with its USB-C port facing the right wall (+X). The Ethernet Shield Rev2 plugs into the Uno-compatible headers at the USB-C end of the GIGA R1, so the shield's RJ45 jack also faces the right wall.

The SD card slot is on the opposite end of the Ethernet Shield from the RJ45, which puts it in the center of the case -- inaccessible from any wall. Open the case to access the SD card if needed.

## Modifications from Original

| Piece | Modification |
|-------|-------------|
| Lid | Display window filled with solid block |
| Base | RJ45 cutout (right wall), terminal cutout (right wall), support post |
| Shelf | Matching RJ45 and terminal cutouts aligned with base |
| Bracket | New part: terminal receptacle mount with M3 holes, dust lip, zip-tie slot |

## Port Layout

Looking at the enclosure from the right wall (+X):

```
+-------------------------------------------+
|                                            |
|      +-----------+   >=6mm   +--------+   |
|      | Terminal  |           |  RJ45  |   |
|      | 18.4x10.4|           | 17x14.5|   |
|      +-----------+           +--------+   |
|                                            |
+-------------------------------------------+
```

The right wall also has existing USB-C and power cutouts from the original case design (retained without modification).

## Building

Requires [OpenSCAD](https://openscad.org/) (`brew install openscad`) and Python 3.10+.

```bash
# Verify cutout dimensions against EAGLE BRD data
make verify

# Render all output STLs
make stls

# Render test slices for quick fit checks
make slices

# Generate cross-section SVGs for paper templates
make cross-sections

# Build everything
make all

# Analyze original STL geometry
make analyze
```

## File Structure

| File | Purpose |
|------|---------|
| `config.scad` | All parameterized dimensions (edit this to tune fitment) |
| `lid.scad` | Modified lid: display window filled |
| `base.scad` | Modified base: port cutouts + support post |
| `shelf.scad` | Modified shelf: aligned port cutouts |
| `terminal_bracket.scad` | New part: internal connector mount |
| `board_model.scad` | Simplified Ethernet Shield 3D model for verification |
| `assembly.scad` | Full assembly view for visual cutout alignment check |
| `test_slices.scad` | Thin wall slices for pre-print fit testing |
| `cross_sections.scad` | 2D projections for 1:1 paper templates |
| `analyze_stl.py` | Extracts bounding boxes from original STLs |
| `verify_alignment.py` | Validates cutout positions against EAGLE BRD data |
| `find_standoffs.py` | Locates mounting standoff positions in base STL |
| `check_positions.py` | Reports board component positions in case coordinates |
| `Makefile` | Build targets for all outputs |
| `original/` | Unmodified STL files from Printables |
| `output/` | Rendered modified STLs, test slices, and SVGs |

## Verification Workflow

### 1. Python validation

```bash
make verify
```

Checks that all cutout dimensions and positions match EAGLE BRD component data within tolerance.

### 2. Visual inspection (OpenSCAD GUI)

Open `assembly.scad` in OpenSCAD to check:
- Cutout alignment (port housings protrude cleanly through cutouts)
- Interference detection (change `mode="interference"` -- should render empty)
- Clearance gap uniformity

### 3. Paper templates

```bash
make cross-sections
```

Print the SVGs at 1:1 scale, cut out, and hold against physical boards.

### 4. Test slice prints

```bash
make slices
```

Print `output/test_slice_rj45_wall.stl` (~5 min) for a quick fit check of the RJ45 and terminal cutouts before committing to a full print.

## Adjusting Fitment

All dimensions are parameterized in `config.scad`. Key parameters to tune:

| Parameter | Default | Description |
|-----------|---------|-------------|
| `board_case_origin_x` / `_y` | 58.7 / -26.6 | Board-to-case position offset (where board X=0 maps in case coords) |
| `rj45_cutout_w` / `rj45_cutout_h` | 17.0 / 14.5 | RJ45 opening size |
| `terminal_cutout_w` / `terminal_cutout_h` | 18.4 / 10.4 | Terminal opening size |
| `rj45_terminal_spacing` | 6.0 | Edge-to-edge spacing between RJ45 and terminal |
| `support_post_diameter` | 10.0 | Support post pad diameter |
| `stacking_header_height` | 8.5 | Distance between GIGA R1 and Ethernet Shield PCBs |

After changing parameters, re-run `make verify` and `make all`.

## Bill of Materials

| Item | Qty | Notes |
|------|-----|-------|
| 4-pos Euroblock plug (3.5 mm pitch) | 1 | Field-wire side |
| 4-pos Euroblock receptacle (3.5 mm pitch) | 1 | Panel side, mounts in bracket |
| 1x4 Molex KK 254 crimp housing (2.54 mm) | 1 | Arduino header end of internal harness |
| Molex KK crimp terminals | 4 | For internal harness |
| 24-26 AWG stranded wire (4 colors) | ~120 mm | Black (GND), White (D2), Green (D3), Red (+5V) |
| M3 x 25 mm bolts | 4 | Case assembly (original hardware) |
| M3 nuts | 4 | Case assembly (original hardware) |
| M3 x 8 mm bolts | 2 | Terminal bracket mounting |
| M3 heat-set inserts | 2 | Terminal bracket |
| TPU pad or 0.5 mm closed-cell foam | 1 | Support post cushion, ~10 mm diameter |
| Bootlace ferrules (optional) | 4 | Euroblock wire termination |

## Assembly Order

1. Install standoffs in base
2. Mount GIGA R1 board
3. Stack Ethernet Shield onto GIGA R1
4. Install terminal receptacle in bracket
5. Secure bracket to enclosure wall (M3 bolts into heat-set inserts)
6. Connect internal harness (Molex KK) to GIGA R1 header pins
7. Apply strain relief (zip tie through bracket slot)
8. Place TPU/foam pad on support post
9. Seat shield shelf
10. Install lid and secure with M3 bolts

## Reference

- [Enclosure specification](../docs/enclosure-specification.md) -- full mechanical and electrical spec
- [Wiring diagram](../docs/wiring.md) -- station-side relay wiring
- [Original case on Printables](https://www.printables.com/model/605051) -- GIGA Display Shield case (starting point)
