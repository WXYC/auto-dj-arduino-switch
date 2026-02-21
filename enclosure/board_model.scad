// board_model.scad — Simplified Ethernet Shield 3D model for verification
//
// Built from EAGLE BRD data. The board's X axis is REVERSED relative
// to the case X axis (board X=0 is at the right wall, board X increases
// toward the left). We use mirror([1,0,0]) to flip the board model.

include <config.scad>

module ethernet_shield_pcb() {
    color(color_board)
        cube([eth_length, eth_width, eth_pcb_thickness]);
}

module rj45_housing() {
    // RJ45 jack housing (UC1)
    // The housing body sits on the PCB extending in the +X direction
    // (into the board). Port face is at x = rj45_board_x.
    color(color_rj45)
        translate([
            rj45_board_x,
            rj45_board_y - rj45_housing_w / 2,
            eth_pcb_thickness
        ])
            cube([rj45_housing_d, rj45_housing_w, rj45_housing_h]);
}

module sd_card_slot() {
    // Micro-SD card slot (X1) — rotated R270
    color(color_sd)
        translate([
            sd_board_x - sd_slot_d / 2,
            sd_board_y - sd_slot_w / 2,
            eth_pcb_thickness
        ])
            cube([sd_slot_d, sd_slot_w, sd_slot_h]);
}

module header_row(x, y, pins, pitch) {
    header_length = pins * pitch;
    header_width = pitch;
    header_height_above = 8.5;
    header_height_below = 3.0;

    color([0.2, 0.2, 0.2, 0.6]) {
        translate([x - header_length / 2, y - header_width / 2, eth_pcb_thickness])
            cube([header_length, header_width, header_height_above]);
        translate([x - header_length / 2, y - header_width / 2, -header_height_below])
            cube([header_length, header_width, header_height_below]);
    }
}

module ethernet_shield() {
    // Complete Ethernet Shield in board-local coordinates.
    // Origin at bottom-left mounting hole.
    ethernet_shield_pcb();
    rj45_housing();
    sd_card_slot();
    header_row(power_header_x, power_header_y, power_header_pins, header_pitch);
    header_row(janalog_header_x, janalog_header_y, janalog_header_pins, header_pitch);
    header_row(jhigh_header_x, jhigh_header_y, jhigh_header_pins, header_pitch);
    header_row(jlow_header_x, jlow_header_y, jlow_header_pins, header_pitch);
}

module ethernet_shield_in_case() {
    // Position the shield in case coordinates.
    // Board X is reversed: case_x = board_case_origin_x - board_x
    // We achieve this by translating to the origin point and mirroring in X.
    translate([board_case_origin_x, board_case_origin_y, eth_pcb_bottom_z])
        mirror([1, 0, 0])
            ethernet_shield();
}

ethernet_shield_in_case();
