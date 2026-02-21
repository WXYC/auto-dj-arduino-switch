// config.scad — Parameterized dimensions for GIGA R1 + Ethernet Shield enclosure
//
// All measurements in millimeters. Values derived from:
// - analyze_stl.py output (case geometry)
// - EAGLE BRD data (Ethernet Shield component positions)
// - KiCad footprint data (GIGA R1 WiFi mounting holes + header positions)
// - Enclosure specification (docs/enclosure-specification.md)
//
// Adjust these parameters and re-render to tune fitment.


// ===================================================================
// STL file paths (relative to this file)
// ===================================================================

original_lid   = "original/lid.stl";
original_base  = "original/base.stl";
original_shelf = "original/shelf.stl";


// ===================================================================
// Case geometry (from analyze_stl.py)
// ===================================================================

// Lid bounding box: centered at origin
lid_x_min = -57.45;
lid_x_max =  57.45;
lid_y_min = -44.45;
lid_y_max =  44.45;
lid_z_min =   0.00;
lid_z_max =   6.45;

// Base bounding box: slightly asymmetric
base_x_min = -57.03;
base_x_max =  57.87;
base_y_min = -44.37;
base_y_max =  44.53;
base_z_min = -16.30;
base_z_max =   3.86;

// Case wall thickness (measured from lid analysis)
wall_thickness = 4.0;

// Display window opening in lid (from detailed lid analysis)
window_x_min = -53.45;
window_x_max =  53.55;
window_y_min = -40.45;
window_y_max =  40.55;
window_z     =   6.45;  // top surface Z

// Window fill block — slightly oversized to ensure clean union
window_fill_margin = 0.5;  // mm extra on each side
window_fill_thickness = 3.5;  // mm thick (fills from inside surface to top)


// ===================================================================
// Board dimensions
// ===================================================================

// Arduino GIGA R1 WiFi (Mega form factor, NOT 76.2mm wide)
giga_length = 101.6;  // mm
giga_width  =  53.34; // mm (same as Uno/Mega width)
giga_pcb_thickness = 1.6;  // mm

// Arduino Ethernet Shield Rev2 (Uno form factor)
eth_length = 68.6;   // mm (X axis in board coords)
eth_width  = 53.3;   // mm (Y axis in board coords)
eth_pcb_thickness = 1.6;  // mm

// The Ethernet Shield is ~33 mm shorter than the GIGA R1
eth_giga_length_diff = giga_length - eth_length;  // ~33.0 mm


// ===================================================================
// Board-to-case coordinate transform
// ===================================================================
//
// ORIENTATION: The GIGA R1's USB-C port faces the +X wall (right wall)
// of the case. Port cutouts on the right wall confirm this.
//
// The board's X axis is REVERSED relative to the case X axis:
//   case_x = board_case_origin_x - board_x
//   case_y = board_case_origin_y + board_y
//
// The Ethernet Shield mounts at the USB-C end (board X = 0 to 68.6).
// The shield's RJ45 port (overhanging at board X ≈ -4.3) faces +X
// (right wall) through the existing side opening.
//
// board_case_origin_x: case X coordinate where board X = 0 maps.
// This is the USB-C end, near the right wall. The USB-C connector
// protrudes slightly past the outer wall for cable access.

board_case_origin_x = 58.7;   // board X=0 → case X=58.7 (right wall area)
board_case_origin_y = -26.6;  // board Y=0 → case Y=-26.6 (bottom edge)

// Convenience: board-to-case coordinate functions as expressions
// For any board position (bx, by):
//   case_x = board_case_origin_x - bx
//   case_y = board_case_origin_y + by

// GIGA R1 extent in case coordinates
giga_case_x_max = board_case_origin_x;                        // ~58.7 (USB-C end, right wall)
giga_case_x_min = board_case_origin_x - giga_length;          // ~-42.9 (JTAG end)
giga_case_y_min = board_case_origin_y;                         // ~-26.6
giga_case_y_max = board_case_origin_y + giga_width;            // ~26.7

// Ethernet Shield extent in case coordinates
// Shield mounts at the USB-C end (board X = 0 to 68.6)
eth_case_x_max = board_case_origin_x;                          // ~58.7 (RJ45 end)
eth_case_x_min = board_case_origin_x - eth_length;             // ~-9.9 (opposite end)
eth_case_y_min = board_case_origin_y;                           // ~-26.6
eth_case_y_max = board_case_origin_y + eth_width;               // ~26.7


// ===================================================================
// Stacking heights (Z coordinates in case space)
// ===================================================================

// Standoff height (GIGA R1 standoffs in base, from find_standoffs.py)
standoff_height = 4.8;  // mm above base floor (measured from STL)

// Base floor Z
base_floor_z = base_z_min;

// GIGA R1 PCB bottom surface Z
giga_pcb_bottom_z = base_floor_z + standoff_height;

// GIGA R1 PCB top surface Z
giga_pcb_top_z = giga_pcb_bottom_z + giga_pcb_thickness;

// Stacking header height (between GIGA R1 top and Ethernet Shield bottom)
stacking_header_height = 8.5;  // mm (standard Arduino stacking headers)

// Ethernet Shield PCB bottom surface Z
eth_pcb_bottom_z = giga_pcb_top_z + stacking_header_height;

// Ethernet Shield PCB top surface Z
eth_pcb_top_z = eth_pcb_bottom_z + eth_pcb_thickness;


// ===================================================================
// Ethernet Shield component positions (from EAGLE BRD)
// All positions relative to board origin (bottom-left mounting hole)
// ===================================================================

// RJ45 jack (UC1)
rj45_board_x  = -4.318;   // mm — overhangs board edge by 4.3 mm
rj45_board_y  = 38.354;   // mm — center position along board edge
rj45_housing_w = 16.0;    // mm — housing width (along Y)
rj45_housing_h = 13.5;    // mm — housing height (along Z)
rj45_housing_d = 21.75;   // mm — housing depth (along X, into board)
rj45_overhang  =  4.318;  // mm — overhang past board edge

// Micro-SD card slot (X1) — rotated 270 degrees
sd_board_x = 63.5;   // mm — near right edge of board
sd_board_y = 19.05;  // mm — center position
sd_slot_w  = 12.0;   // mm — slot width
sd_slot_h  =  2.5;   // mm — slot height
sd_slot_d  = 14.0;   // mm — slot depth

// Header positions (for board model)
power_header_x = 36.83;
power_header_y = 2.54;
power_header_pins = 8;

janalog_header_x = 57.15;
janalog_header_y = 2.54;
janalog_header_pins = 6;

jhigh_header_x = 30.226;
jhigh_header_y = 50.8;
jhigh_header_pins = 10;

jlow_header_x = 54.61;
jlow_header_y = 50.8;
jlow_header_pins = 8;

header_pitch = 2.54;  // mm


// ===================================================================
// Port cutout dimensions (with clearances)
// ===================================================================

// RJ45 cutout: housing + 0.5mm clearance per side
rj45_cutout_w = 17.0;   // mm (16.0 + 0.5*2)
rj45_cutout_h = 14.5;   // mm (13.5 + 0.5*2)
rj45_cutout_depth = 10.0;  // mm — depth through wall (oversized)

// SD card cutout: REMOVED — SD slot is in the center of the case,
// inaccessible from any side wall. The Ethernet Shield is 33 mm
// shorter than the GIGA R1, so the SD end sits ~35 mm from the
// nearest wall. Open the case to access the SD card if needed.

// Terminal cutout: Euroblock envelope + 0.2mm printer tolerance per side
terminal_cutout_w = 18.4;  // mm (18.0 + 0.2*2)
terminal_cutout_h = 10.4;  // mm (10.0 + 0.2*2)
terminal_cutout_depth = 10.0;  // mm
terminal_corner_r = 1.0;  // mm (optional corner radius)

// Spacing between RJ45 and terminal cutouts
rj45_terminal_spacing = 6.0;  // mm edge-to-edge minimum


// ===================================================================
// Port cutout positions in case coordinates
// ===================================================================

// RJ45 is on the RIGHT wall (+X) of the case.
// The RJ45 port faces +X in case coordinates (board -X maps to case +X).
// The right wall already has a large opening from the Display Shield
// design. We cut a specific RJ45-sized opening.
//
// RJ45 board position: (-4.318, 38.354) in board coords.
// In case coords: x = board_case_origin_x - (-4.318) = ~63.0 (past outer wall — port face)
// Y center: board_case_origin_y + 38.354 = ~11.75

rj45_case_y = board_case_origin_y + rj45_board_y;
rj45_case_z = eth_pcb_top_z + rj45_housing_h / 2;

// Terminal cutout: same wall as RJ45 (right wall, +X), below RJ45.
// The terminal should be offset in Y from RJ45 to maintain >=6mm spacing.
terminal_case_y = rj45_case_y - rj45_cutout_w / 2 - rj45_terminal_spacing - terminal_cutout_w / 2;
terminal_case_z = rj45_case_z;  // vertically centered with RJ45

// Wall X position for cutouts (right wall)
cutout_wall_x = base_x_max;  // right wall (+X)


// ===================================================================
// Support post dimensions
// ===================================================================

// Contact point on Ethernet Shield underside (in board coordinates)
// Safe zone: x = 10-14 mm, y = 32-36 mm (from EAGLE BRD analysis)
// The free (unsupported) end of the Ethernet Shield is at board X = 0
// (RJ45 side), which is at the +X end of the case (near the right wall).
support_post_board_x = 12.0;   // center of safe zone
support_post_board_y = 34.0;   // center of safe zone
support_post_diameter = 10.0;  // mm flat pad diameter

// Post position in case coordinates (board X reversed)
support_post_case_x = board_case_origin_x - support_post_board_x;
support_post_case_y = board_case_origin_y + support_post_board_y;

// Post height: from base floor to Ethernet Shield underside
support_post_height = eth_pcb_bottom_z - base_floor_z - 0.5;  // 0.5mm gap for TPU/foam pad
support_post_case_z_bottom = base_floor_z;


// ===================================================================
// Terminal bracket dimensions
// ===================================================================

bracket_thickness = 2.0;    // mm plate thickness
bracket_m3_hole_d = 3.2;   // mm (M3 clearance hole)
bracket_m3_spacing = 24.0; // mm between M3 holes (straddles terminal)
bracket_dust_lip = 2.0;    // mm internal dust lip depth
bracket_zip_slot_w = 4.0;  // mm zip-tie slot width
bracket_zip_slot_h = 2.0;  // mm zip-tie slot height


// ===================================================================
// Rendering quality
// ===================================================================

$fn = 60;  // circle resolution (increase for final renders)


// ===================================================================
// Colors for visualization
// ===================================================================

color_case = [0.3, 0.3, 0.3, 0.4];   // dark gray, semi-transparent
color_board = [0.0, 0.5, 0.0, 0.7];   // green PCB
color_rj45 = [0.7, 0.7, 0.7, 0.9];   // silver
color_sd = [0.2, 0.2, 0.2, 0.9];     // dark
color_post = [0.8, 0.4, 0.0, 0.9];   // orange
color_bracket = [0.6, 0.6, 0.8, 0.9]; // light blue
