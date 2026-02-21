// config.scad — Parameterized dimensions for GIGA R1 + Ethernet Shield enclosure
//
// All measurements in millimeters. Values derived from:
// - analyze_stl.py output (case geometry)
// - EAGLE BRD data (Ethernet Shield component positions)
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
// The window is the uncovered region on the top face
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

// Arduino GIGA R1 WiFi
giga_length = 101.5;  // mm (X axis in board coords)
giga_width  =  76.2;  // mm (Y axis in board coords)
giga_pcb_thickness = 1.6;  // mm

// Arduino Ethernet Shield Rev2 (Uno form factor)
eth_length = 68.6;   // mm (X axis in board coords)
eth_width  = 53.3;   // mm (Y axis in board coords)
eth_pcb_thickness = 1.6;  // mm

// The Ethernet Shield is ~33 mm shorter than the GIGA R1
eth_giga_length_diff = giga_length - eth_length;  // ~32.9 mm


// ===================================================================
// Board-to-case coordinate transform
// ===================================================================

// The GIGA R1 board origin (bottom-left mounting hole) mapped to case
// coordinates. This positions the board inside the case.
//
// The GIGA R1 mounts on standoffs in the base. The Ethernet Shield
// plugs into the Uno-compatible headers on the GIGA R1.
//
// IMPORTANT: These offsets need verification against the actual STL
// mounting hole positions. Adjust after visual inspection in OpenSCAD.
//
// The GIGA R1's USB-C port faces the +X wall of the case.
// Board X=0 (mounting hole end) maps to the -X side of the case.

// Board origin in case coordinates
giga_board_origin_x = base_x_min + wall_thickness + 2.0;  // ~-51.03
giga_board_origin_y = (base_y_min + base_y_max) / 2 - giga_width / 2;  // centered in Y

// The Ethernet Shield sits at the same X origin as the GIGA R1's
// Uno-compatible headers. On the GIGA R1, these headers start at the
// board origin end (X=0). The Ethernet Shield's origin (mounting hole)
// aligns with the GIGA R1's Uno mounting hole.
eth_board_origin_x = giga_board_origin_x;
eth_board_origin_y = giga_board_origin_y;


// ===================================================================
// Stacking heights (Z coordinates in case space)
// ===================================================================

// Standoff height (GIGA R1 standoffs in base)
standoff_height = 5.0;  // mm above base floor

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
rj45_housing_w = 16.0;    // mm — housing width
rj45_housing_h = 13.5;    // mm — housing height
rj45_housing_d = 21.75;   // mm — housing depth (into board)
rj45_overhang  =  4.318;  // mm — overhang past board edge

// Micro-SD card slot (X1) — rotated 270 degrees
sd_board_x = 63.5;   // mm — at right edge of board
sd_board_y = 19.05;  // mm — center position
sd_slot_w  = 12.0;   // mm — slot width
sd_slot_h  =  2.5;   // mm — slot height
sd_slot_d  = 14.0;   // mm — slot depth

// Header positions (for board model)
// POWER header: (36.83, 2.54), 1x8, 2.54mm pitch
power_header_x = 36.83;
power_header_y = 2.54;
power_header_pins = 8;

// JANALOG header: (57.15, 2.54), 1x6, 2.54mm pitch
janalog_header_x = 57.15;
janalog_header_y = 2.54;
janalog_header_pins = 6;

// JHIGH header: (30.226, 50.8), 1x10, 2.54mm pitch, R180
jhigh_header_x = 30.226;
jhigh_header_y = 50.8;
jhigh_header_pins = 10;

// JLOW header: (54.61, 50.8), 1x8, 2.54mm pitch, R180
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

// SD card cutout: slot + 0.5mm clearance per side
sd_cutout_w = 13.0;   // mm (12.0 + 0.5*2)
sd_cutout_h =  3.5;   // mm (2.5 + 0.5*2)
sd_cutout_depth = 10.0;  // mm

// SD card chamfer for fingernail access
sd_chamfer_depth = 1.5;  // mm
sd_chamfer_height = 2.0; // mm below the slot

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

// RJ45 is on the left wall (-X) of the case.
// The RJ45 jack overhangs the board edge, facing -X.
// Center Y: board_origin_y + rj45_board_y
// Center Z: eth_pcb_top_z + rj45_housing_h/2 (mounted on top of PCB)

rj45_case_y = eth_board_origin_y + rj45_board_y;
rj45_case_z = eth_pcb_top_z + rj45_housing_h / 2;

// Terminal cutout: same wall as RJ45, to the right (higher Y),
// vertically centered with RJ45
terminal_case_y = rj45_case_y + rj45_cutout_w / 2 + rj45_terminal_spacing + terminal_cutout_w / 2;
terminal_case_z = rj45_case_z;  // vertically centered with RJ45

// SD card is on the right wall (+X) of the case (or opposite short wall).
// The SD slot faces +X direction.
// Center Y: board_origin_y + sd_board_y
// Center Z: eth_pcb_top_z + sd_slot_h/2

sd_case_y = eth_board_origin_y + sd_board_y;
sd_case_z = eth_pcb_top_z + sd_slot_h / 2;

// Wall X positions for cutout placement
rj45_wall_x = base_x_min;      // left wall (RJ45 + terminal)
sd_wall_x   = base_x_max;      // right wall (SD card)


// ===================================================================
// Support post dimensions
// ===================================================================

// Contact point on Ethernet Shield underside (in board coordinates)
// Safe zone: x = 10-14 mm, y = 32-36 mm (from EAGLE BRD analysis)
support_post_board_x = 12.0;   // center of safe zone
support_post_board_y = 34.0;   // center of safe zone
support_post_diameter = 10.0;  // mm flat pad diameter

// Post position in case coordinates
support_post_case_x = eth_board_origin_x + support_post_board_x;
support_post_case_y = eth_board_origin_y + support_post_board_y;

// Post height: from base floor to Ethernet Shield underside
// (minus small gap for compliant pad)
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
