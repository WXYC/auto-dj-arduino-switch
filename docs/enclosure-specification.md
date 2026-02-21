# Arduino GIGA R1 WiFi + Ethernet Shield Rev2

# Sealed Enclosure Modification & Wiring Specification

------------------------------------------------------------------------

## 1. Project Overview

This document defines the mechanical, electrical, and assembly
specifications for modifying an existing 3D-printed **base + lid
enclosure** originally designed for the GIGA Display Shield so that it
supports:

-   Arduino GIGA R1 WiFi
-   Arduino Ethernet Shield Rev2 (W5500 + SD)
-   A sealed design (no open side slots)
-   A 4-wire pluggable screw terminal connection
-   Dust-resistant configuration
-   2--3 mm wall thickness

The enclosure will expose only:

-   RJ45 Ethernet jack
-   SD card slot
-   USB-C port
-   Power input
-   4-position pluggable terminal connector

------------------------------------------------------------------------

# 2. Electrical Interface Specification

## 2.1 Signals (4-wire breakout)

  Terminal Pin   Signal   Direction   Notes
  -------------- -------- ----------- ------------------------------------------
  1              GND      Common      Shared reference
  2              D2       Input       Relay dry-contact input (`INPUT_PULLUP`)
  3              D3       Output      LED indicator output
  4              +5V      Power       Reserved for future use

### Recommended Wire Colors

  Signal   Color
  -------- -------
  GND      Black
  D2       White
  D3       Green
  +5V      Red

------------------------------------------------------------------------

# 3. Connector Specification

## 3.1 Panel Connector Type

4-position pluggable screw terminal block

-   Pitch: 3.5 mm
-   Type: Plug + receptacle (Euroblock style)
-   Field wires land in removable plug
-   Receptacle mounted internally

### Terminal Cutout Size (starting tolerance envelope)

Width: 18.0 mm
Height: 10.0 mm
Corner radius (optional): 1.0--1.5 mm

Add printer tolerance:
+0.2 to +0.4 mm per side if needed

------------------------------------------------------------------------

# 4. Enclosure Design Requirements

## 4.1 Wall Thickness

-   2.0--3.0 mm
-   Minimum ligament between adjacent cutouts: 3.0 mm
-   Preferred spacing between RJ45 and terminal cutout: 6.0 mm
    edge-to-edge

------------------------------------------------------------------------

## 4.2 Terminal Plug Placement

Location:
On the same exterior wall as the RJ45 connector.
Placed to the right-hand side of the RJ45 when viewed externally.

### Alignment Rules

1.  Vertically align the terminal cutout center with the RJ45 center.
2.  Maintain 6.0 mm minimum spacing between RJ45 and terminal cutouts.
3.  Ensure >=3.0 mm wall material to outer edges.

------------------------------------------------------------------------

## 4.3 Enclosure Construction

The enclosure is a three-piece stacking design:

1. **Base** (solid bottom) -- the main case body; the GIGA R1 mounts here on standoffs
2. **Shield shelf** (bottom with holes) -- stacks inside the base as an internal platform; the Ethernet Shield's header pass-through pins protrude through the holes
3. **Lid** (top) -- closes the enclosure; display window is filled solid

The existing USB-C and power input cutouts from the original GIGA R1 case design are retained without modification.

------------------------------------------------------------------------

# 5. Ethernet Shield Cutouts

## 5.1 RJ45 Cutout

-   Must accommodate overhang beyond PCB edge
-   Add +0.5 mm clearance per side
-   Ensure full latch access
-   **Cutout dimensions**: 17.0 x 14.5 mm (16.0 x 13.5 mm housing + 0.5 mm clearance/side)
-   **Position**: right wall (+X), where the Ethernet Shield's RJ45 jack faces. The GIGA R1's USB-C port is on this same wall. The RJ45 jack at board position (-4.318, 38.354) overhangs the board edge by 4.3 mm, protruding through the right wall.
-   Must accommodate 4.3 mm board-edge overhang

## 5.2 SD Card Slot Access

The Ethernet Shield's micro-SD slot is at board position (63.5, 19.05), near the end of the shield opposite the RJ45. Because the Ethernet Shield (68.6 mm) is ~33 mm shorter than the GIGA R1 (101.6 mm), this end of the shield sits in the center of the case, ~35 mm from the nearest wall.

**No SD card cutout is provided.** The SD card is inaccessible from any side wall in this enclosure. Open the case to access the SD card if needed. (The SD card is typically only used for debug logging, not required for normal operation.)

## 5.3 Terminal Cutout

-   **Cutout dimensions**: 18.4 x 10.4 mm (18.0 x 10.0 mm Euroblock envelope + 0.2 mm printer tolerance/side)
-   **Position**: right wall (+X), same wall as RJ45, offset in Y with >=6 mm edge-to-edge spacing, vertically centered with RJ45 cutout

------------------------------------------------------------------------

# 6. Lid Modifications

The current lid STL includes a display window.

Required Changes:

-   Remove display opening entirely
-   Replace with solid surface
-   Maintain same perimeter geometry

------------------------------------------------------------------------

# 7. Internal Terminal Mounting Bracket

## 7.1 Requirements

-   Align terminal receptacle behind cutout
-   Provide strain relief
-   Secure via two M3 fasteners
-   Integrate zip-tie anchor

Bracket Feature List:

-   Two M3 mounting bosses (heat-set inserts recommended)
-   2--3 mm thick
-   2 mm internal dust lip around cutout perimeter
-   Zip tie slot (>=4 mm wide)

------------------------------------------------------------------------

# 8. Internal Wiring Harness

## 8.1 Arduino End

-   1x4 2.54 mm crimp housing
-   Crimped terminals (Molex KK or equivalent)
-   No loose Dupont jumpers

## 8.2 Harness Length

-   80--120 mm
-   Include service loop
-   Anchor before reaching header pins

## 8.3 Wire Spec

-   24--26 AWG stranded
-   Optional: bootlace ferrules for terminal plug side

------------------------------------------------------------------------

# 9. Mechanical Clearance Requirements

| Parameter | Minimum |
|-----------|---------|
| Internal stack height | >=45 mm |
| PCB-to-wall running clearance | >=1.0-1.5 mm per side (>=2.0 mm for tight/warping printers) |
| Cutout-to-component interference clearance | >=3.0 mm from cutout edge to nearest internal component |
| Cutout spacing | >=6 mm |
| Structural web thickness | >=3 mm |

------------------------------------------------------------------------

## 9.1 Ethernet Shield Support Post

The Ethernet Shield (68.6 mm) is ~33 mm shorter than the GIGA R1 (101.5 mm). Stackable headers support the shield along both long edges, but the free end (RJ45 side, x = 0-19 mm) cantileveres ~19 mm unsupported. A printed support post from the base provides additional rigidity.

Requirements:

- Contact point: underside area with no exposed pads, traces, or vias -- **x = 10-14 mm, y = 32-36 mm** (verified clear via EAGLE BRD bottom copper analysis, >=2 mm margin to nearest via)
- Flat pad: 8-12 mm diameter
- Compliant layer: TPU pad or 0.5-1 mm closed-cell foam to avoid point loading
- Height: matched to shield underside (base floor + Giga standoff + Giga PCB + stacking clearance); minimal preload
- Electrical safety: >=1.0 mm clearance to all solder joints; no contact with vias

------------------------------------------------------------------------

# 10. Assembly Order

1.  Install standoffs in base.
2.  Mount GIGA board.
3.  Stack Ethernet Shield.
4.  Install terminal receptacle in bracket.
5.  Secure bracket to enclosure.
6.  Connect internal harness to GIGA header.
7.  Apply strain relief.
8.  Install lid and secure.

------------------------------------------------------------------------

# 11. Dust Mitigation Strategy

To improve dust resistance:

-   Use internal dust lip (2 mm depth)
-   Maintain tight cutout tolerances
-   Optionally add closed-cell foam strip behind terminal face
-   Avoid side ventilation slots
-   Use fully closed lid

------------------------------------------------------------------------

# 12. Signal Integrity Considerations

-   Signals are low-speed digital
-   No high-frequency lines on breakout
-   Short internal harness minimizes noise
-   No shielding required

------------------------------------------------------------------------

# 13. Recommended CAD Workflow

1.  Import enclosure STL.
2.  Import Ethernet Shield board file.
3.  Align via mounting holes.
4.  Sketch RJ45 and SD cutouts from board geometry.
5.  Apply clearance offsets.
6.  Add terminal cutout using placement rules.
7.  Integrate bracket.
8.  Export updated STL.

------------------------------------------------------------------------

# 14. Final Design Summary

You will have:

-   Sealed base + lid enclosure
-   RJ45 + SD access
-   Right-side 4-wire pluggable terminal connector
-   Internal strain relief
-   2--3 mm wall thickness
-   No open side header exposure
-   Dust-resistant configuration
