# Enclosure: Arduino GIGA R1 WiFi + Ethernet Shield Rev2

Modified 3D-printed enclosure for the auto DJ Arduino switch. Based on the [GIGA Display Shield case](https://www.printables.com/model/605051) (Printables model 605051), adapted to house an Ethernet Shield Rev2 instead of the Display Shield.

## Design Overview

Three-piece stacking enclosure:

1. **Base** -- main case body with GIGA R1 standoffs. Modified with RJ45, SD card, and terminal cutouts plus an internal support post.
2. **Shield shelf** -- internal platform that stacks inside the base. Modified with matching port cutouts.
3. **Lid** -- top cover. Display window filled solid for dust resistance.

Additionally:

4. **Terminal bracket** -- new part designed from scratch. Internal mount for the Euroblock screw terminal receptacle with strain relief.

## Modifications from Original

| Piece | Modification |
|-------|-------------|
| Lid | Display window filled with solid block |
| Base | RJ45 cutout (left wall), SD cutout (right wall), terminal cutout (left wall), support post |
| Shelf | Matching RJ45, SD, and terminal cutouts aligned with base |
| Bracket | New part: terminal receptacle mount with M3 holes, dust lip, zip-tie slot |

## Port Layout

Looking at the enclosure from the left wall (-X):

```
+-------------------------------------------+
|                                            |
|   +--------+   >=6mm   +-----------+      |
|   |  RJ45  |           | Terminal  |      |
|   | 17x14.5|           | 18.4x10.4|      |
|   +--------+           +-----------+      |
|                                            |
+-------------------------------------------+
```

The SD card slot (13.0 x 3.5 mm) is on the opposite wall (+X), with a fingernail chamfer below.

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
| `test_slices.scad` | Thin wall slices for pre-print fit testing |
| `cross_sections.scad` | 2D projections for 1:1 paper templates |
| `analyze_stl.py` | Extracts bounding boxes from original STLs |
| `verify_alignment.py` | Validates cutout positions against EAGLE BRD data |
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

Open `board_model.scad` overlaid inside the case to check:
- Cutout alignment (port housings protrude cleanly through cutouts)
- Interference detection (`intersection(case, board)` renders empty)
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

Print `output/test_slice_rj45_wall.stl` (~5 min) and `output/test_slice_sd_wall.stl` (~3 min) for quick fit checks before committing to a full print.

## Adjusting Fitment

All dimensions are parameterized in `config.scad`. Key parameters to tune:

| Parameter | Default | Description |
|-----------|---------|-------------|
| `rj45_cutout_w` / `rj45_cutout_h` | 17.0 / 14.5 | RJ45 opening size |
| `sd_cutout_w` / `sd_cutout_h` | 13.0 / 3.5 | SD card opening size |
| `terminal_cutout_w` / `terminal_cutout_h` | 18.4 / 10.4 | Terminal opening size |
| `rj45_terminal_spacing` | 6.0 | Edge-to-edge spacing between RJ45 and terminal |
| `support_post_diameter` | 10.0 | Support post pad diameter |
| `eth_board_origin_x` / `_y` | (calculated) | Board-to-case position offset |
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
