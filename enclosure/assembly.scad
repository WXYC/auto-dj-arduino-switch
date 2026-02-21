// assembly.scad — Full assembly view for visual cutout alignment verification
//
// Shows the modified case pieces (semi-transparent) with the Ethernet
// Shield board model positioned inside at the correct stacking height.
//
// Usage:
//   Open in OpenSCAD GUI for visual inspection.
//   Render interference: openscad -D 'mode="interference"' -o output/interference.stl assembly.scad

include <config.scad>

mode = "assembly";  // "assembly" or "interference"

// --- Import modified pieces (rendered from source .scad files) ---

module mod_lid() {
    color(color_case)
        import("output/lid.stl", convexity=10);
}

module mod_base() {
    color(color_case)
        import("output/base.stl", convexity=10);
}

module mod_shelf() {
    color(color_case)
        import("output/shelf.stl", convexity=10);
}

// --- Board model (inline, using reversed X mapping) ---

module eth_board_in_case() {
    translate([board_case_origin_x, board_case_origin_y, eth_pcb_bottom_z])
        mirror([1, 0, 0]) {
            // PCB
            color(color_board)
                cube([eth_length, eth_width, eth_pcb_thickness]);
            // RJ45 housing
            color(color_rj45)
                translate([rj45_board_x, rj45_board_y - rj45_housing_w / 2, eth_pcb_thickness])
                    cube([rj45_housing_d, rj45_housing_w, rj45_housing_h]);
            // SD slot
            color(color_sd)
                translate([sd_board_x - sd_slot_d / 2, sd_board_y - sd_slot_w / 2, eth_pcb_thickness])
                    cube([sd_slot_d, sd_slot_w, sd_slot_h]);
        }
}

// --- Case walls (for interference detection) ---

module case_walls() {
    union() {
        import("output/base.stl", convexity=10);
        import("output/shelf.stl", convexity=10);
        import("output/lid.stl", convexity=10);
    }
}

module board_solids() {
    translate([board_case_origin_x, board_case_origin_y, eth_pcb_bottom_z])
        mirror([1, 0, 0]) {
            cube([eth_length, eth_width, eth_pcb_thickness]);
            translate([rj45_board_x, rj45_board_y - rj45_housing_w / 2, eth_pcb_thickness])
                cube([rj45_housing_d, rj45_housing_w, rj45_housing_h]);
            translate([sd_board_x - sd_slot_d / 2, sd_board_y - sd_slot_w / 2, eth_pcb_thickness])
                cube([sd_slot_d, sd_slot_w, sd_slot_h]);
        }
}

// --- Render modes ---

module assembly_view() {
    mod_base();
    mod_shelf();
    mod_lid();
    eth_board_in_case();
}

module interference_check() {
    color([1, 0, 0])
        intersection() {
            case_walls();
            board_solids();
        }
}

if (mode == "assembly") {
    assembly_view();
} else if (mode == "interference") {
    interference_check();
}
