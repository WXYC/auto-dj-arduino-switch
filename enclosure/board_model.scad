// board_model.scad — Simplified Ethernet Shield 3D model for verification
//
// A simplified 3D representation of the Arduino Ethernet Shield Rev2,
// built from EAGLE BRD data. Used for:
// - Visual cutout alignment verification (overlay inside case)
// - Interference detection (intersection with case walls)
// - Clearance gap visualization

include <config.scad>

module ethernet_shield_pcb() {
    // PCB outline (simplified rectangle — actual board has a notched corner)
    color(color_board)
        cube([eth_length, eth_width, eth_pcb_thickness]);
}

module rj45_housing() {
    // RJ45 jack housing (UC1)
    // Position: (-4.318, 38.354) in board coordinates
    // The housing overhangs the left board edge
    color(color_rj45)
        translate([
            rj45_board_x - rj45_housing_d + rj45_overhang,
            rj45_board_y - rj45_housing_w / 2,
            eth_pcb_thickness
        ])
            cube([rj45_housing_d, rj45_housing_w, rj45_housing_h]);
}

module sd_card_slot() {
    // Micro-SD card slot (X1) — rotated R270
    // Position: (63.5, 19.05) in board coordinates
    color(color_sd)
        translate([
            sd_board_x - sd_slot_d / 2,
            sd_board_y - sd_slot_w / 2,
            eth_pcb_thickness
        ])
            cube([sd_slot_d, sd_slot_w, sd_slot_h]);
}

module header_row(x, y, pins, pitch) {
    // Simplified header pin volume (rectangular block)
    header_length = pins * pitch;
    header_width = pitch;
    header_height_above = 8.5;  // stacking header total height above PCB
    header_height_below = 3.0;  // pin protrusion below PCB

    color([0.2, 0.2, 0.2, 0.6]) {
        // Above PCB (header housing + pins)
        translate([
            x - header_length / 2,
            y - header_width / 2,
            eth_pcb_thickness
        ])
            cube([header_length, header_width, header_height_above]);

        // Below PCB (pin protrusion through to GIGA R1)
        translate([
            x - header_length / 2,
            y - header_width / 2,
            -header_height_below
        ])
            cube([header_length, header_width, header_height_below]);
    }
}

module ethernet_shield() {
    // Complete Ethernet Shield model in board-local coordinates.
    // Origin at bottom-left mounting hole.
    ethernet_shield_pcb();
    rj45_housing();
    sd_card_slot();

    // Stacking headers
    header_row(power_header_x, power_header_y, power_header_pins, header_pitch);
    header_row(janalog_header_x, janalog_header_y, janalog_header_pins, header_pitch);
    header_row(jhigh_header_x, jhigh_header_y, jhigh_header_pins, header_pitch);
    header_row(jlow_header_x, jlow_header_y, jlow_header_pins, header_pitch);
}

module ethernet_shield_in_case() {
    // Ethernet Shield positioned in case coordinates at the correct
    // stacking height.
    translate([eth_board_origin_x, eth_board_origin_y, eth_pcb_bottom_z])
        ethernet_shield();
}

// When rendered directly, show the board in case position
ethernet_shield_in_case();
