// base.scad — Modified base with port cutouts and support post
//
// Takes the original Display Shield base and adds:
// - RJ45 cutout on the left wall (-X)
// - Terminal cutout on the left wall (-X), right of RJ45
// - SD card cutout on the right wall (+X)
// - Support post rising from the base floor

include <config.scad>

module original_base() {
    import(original_base, convexity=10);
}

module rj45_cutout() {
    // RJ45 cutout through the left wall (-X).
    // Positioned using case coordinates derived from EAGLE BRD data.
    translate([
        rj45_wall_x - rj45_cutout_depth / 2,
        rj45_case_y,
        rj45_case_z
    ])
        cube([rj45_cutout_depth, rj45_cutout_w, rj45_cutout_h], center=true);
}

module sd_cutout() {
    // SD card cutout through the right wall (+X).
    translate([
        sd_wall_x + sd_cutout_depth / 2,
        sd_case_y,
        sd_case_z
    ])
        cube([sd_cutout_depth, sd_cutout_w, sd_cutout_h], center=true);
}

module sd_chamfer() {
    // Chamfer below the SD slot for fingernail access.
    translate([
        sd_wall_x + sd_chamfer_depth / 2,
        sd_case_y,
        sd_case_z - sd_cutout_h / 2 - sd_chamfer_height / 2
    ])
        // Angled cut: wider at the outside, narrower inside
        hull() {
            cube([sd_chamfer_depth, sd_cutout_w, 0.01], center=true);
            translate([0, 0, -sd_chamfer_height])
                cube([0.01, sd_cutout_w - 2, 0.01], center=true);
        }
}

module terminal_cutout() {
    // Terminal cutout through the left wall (-X), right of RJ45.
    // Vertically centered with RJ45 cutout.
    if (terminal_corner_r > 0) {
        translate([
            rj45_wall_x - terminal_cutout_depth / 2,
            terminal_case_y,
            terminal_case_z
        ])
            // Rounded rectangle cutout
            rotate([0, 90, 0])
                linear_extrude(terminal_cutout_depth, center=true)
                    offset(r=terminal_corner_r)
                        square([
                            terminal_cutout_h - 2 * terminal_corner_r,
                            terminal_cutout_w - 2 * terminal_corner_r
                        ], center=true);
    } else {
        translate([
            rj45_wall_x - terminal_cutout_depth / 2,
            terminal_case_y,
            terminal_case_z
        ])
            cube([terminal_cutout_depth, terminal_cutout_w, terminal_cutout_h], center=true);
    }
}

module support_post() {
    // Cylindrical support post from base floor to Ethernet Shield
    // underside. Flat pad on top for TPU/foam cushion.
    translate([support_post_case_x, support_post_case_y, base_floor_z])
        cylinder(
            h = support_post_height,
            d = support_post_diameter,
            $fn = $fn
        );
}

module modified_base() {
    union() {
        difference() {
            original_base();

            // Cut port openings
            rj45_cutout();
            sd_cutout();
            sd_chamfer();
            terminal_cutout();
        }

        // Add support post
        support_post();
    }
}

// Render the modified base
modified_base();
