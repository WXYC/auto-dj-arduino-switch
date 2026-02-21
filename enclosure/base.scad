// base.scad — Modified base with port cutouts and support post
//
// Takes the original Display Shield base and adds:
// - RJ45 cutout on the right wall (+X)
// - Terminal cutout on the right wall (+X), offset from RJ45
// - Support post rising from the base floor
//
// No SD card cutout — the SD slot is in the center of the case
// (inaccessible from any wall; open the case to access).

include <config.scad>

module original_base() {
    import(original_base, convexity=10);
}

module rj45_cutout() {
    // RJ45 cutout through the right wall (+X).
    // The RJ45 port faces +X in case coordinates.
    translate([
        cutout_wall_x + rj45_cutout_depth / 2,
        rj45_case_y,
        rj45_case_z
    ])
        cube([rj45_cutout_depth, rj45_cutout_w, rj45_cutout_h], center=true);
}

module terminal_cutout() {
    // Terminal cutout through the right wall (+X), offset from RJ45.
    if (terminal_corner_r > 0) {
        translate([
            cutout_wall_x + terminal_cutout_depth / 2,
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
            cutout_wall_x + terminal_cutout_depth / 2,
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
            rj45_cutout();
            terminal_cutout();
        }
        support_post();
    }
}

modified_base();
