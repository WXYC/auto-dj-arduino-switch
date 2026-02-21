// shelf.scad — Modified shield shelf with port cutouts
//
// Takes the original Display Shield shelf (internal platform) and adds
// port cutouts aligned with the base cutouts. Together, the base and
// shelf cutouts form complete port openings through the full wall height.

include <config.scad>

module original_shelf() {
    import(original_shelf, convexity=10);
}

module rj45_cutout_shelf() {
    // RJ45 cutout through the left wall portion of the shelf.
    // Same Y/Z position as the base cutout.
    translate([
        rj45_wall_x - rj45_cutout_depth / 2,
        rj45_case_y,
        rj45_case_z
    ])
        cube([rj45_cutout_depth, rj45_cutout_w, rj45_cutout_h], center=true);
}

module sd_cutout_shelf() {
    // SD card cutout through the right wall portion of the shelf.
    translate([
        sd_wall_x + sd_cutout_depth / 2,
        sd_case_y,
        sd_case_z
    ])
        cube([sd_cutout_depth, sd_cutout_w, sd_cutout_h], center=true);
}

module terminal_cutout_shelf() {
    // Terminal cutout through the left wall portion of the shelf.
    if (terminal_corner_r > 0) {
        translate([
            rj45_wall_x - terminal_cutout_depth / 2,
            terminal_case_y,
            terminal_case_z
        ])
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

module modified_shelf() {
    difference() {
        original_shelf();

        // Cut port openings (aligned with base cutouts)
        rj45_cutout_shelf();
        sd_cutout_shelf();
        terminal_cutout_shelf();
    }
}

// Render the modified shelf
modified_shelf();
