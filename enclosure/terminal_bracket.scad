// terminal_bracket.scad — Internal terminal mounting bracket (new part)
//
// Designed from scratch. Mounts behind the terminal wall cutout to
// hold the Euroblock receptacle in place with strain relief.
//
// Features:
// - 2 mm thick mounting plate
// - Two M3 through-holes for heat-set inserts
// - 2 mm internal dust lip around cutout perimeter
// - Zip-tie slot for harness strain relief

include <config.scad>

// Bracket dimensions derived from terminal cutout
bracket_width  = terminal_cutout_w + 2 * bracket_dust_lip + 8.0;  // total width with mounting ears
bracket_height = terminal_cutout_h + 2 * bracket_dust_lip + 8.0;  // total height with mounting ears

// Terminal opening in bracket (matches Euroblock receptacle)
bracket_opening_w = 18.0;  // Euroblock receptacle width
bracket_opening_h = 10.0;  // Euroblock receptacle height

module bracket_plate() {
    // Main mounting plate
    difference() {
        // Plate body
        cube([bracket_thickness, bracket_width, bracket_height], center=true);

        // Terminal opening
        cube([bracket_thickness + 1, bracket_opening_w, bracket_opening_h], center=true);

        // M3 mounting holes (one above, one below the terminal)
        for (dz = [-bracket_height / 2 + 4, bracket_height / 2 - 4]) {
            translate([0, 0, dz])
                rotate([0, 90, 0])
                    cylinder(d=bracket_m3_hole_d, h=bracket_thickness + 1, center=true, $fn=24);
        }

        // Zip-tie slot (below the terminal opening, offset to one side)
        translate([0, bracket_width / 4, -bracket_opening_h / 2 - bracket_dust_lip - 1])
            cube([bracket_thickness + 1, bracket_zip_slot_w, bracket_zip_slot_h], center=true);
    }
}

module dust_lip() {
    // Internal dust lip — a raised border around the terminal opening
    // on the inside face. Prevents dust ingress around the connector.
    lip_outer_w = bracket_opening_w + 2 * bracket_dust_lip;
    lip_outer_h = bracket_opening_h + 2 * bracket_dust_lip;

    translate([bracket_thickness / 2, 0, 0])
        difference() {
            // Lip body
            cube([bracket_dust_lip, lip_outer_w, lip_outer_h], center=true);
            // Opening through lip
            cube([bracket_dust_lip + 1, bracket_opening_w, bracket_opening_h], center=true);
        }
}

module terminal_bracket() {
    color(color_bracket) {
        union() {
            bracket_plate();
            dust_lip();
        }
    }
}

module terminal_bracket_in_case() {
    // Bracket positioned behind the terminal wall cutout.
    // The bracket's front face sits against the inside of the wall.
    translate([
        rj45_wall_x + wall_thickness + bracket_thickness / 2,
        terminal_case_y,
        terminal_case_z
    ])
        terminal_bracket();
}

// When rendered directly, show the bracket in case position
terminal_bracket_in_case();
