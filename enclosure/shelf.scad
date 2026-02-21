// shelf.scad — Modified shield shelf with port cutouts
//
// Takes the original Display Shield shelf and adds port cutouts aligned
// with the base cutouts on the right wall (+X).

include <config.scad>

module original_shelf() {
    import(original_shelf, convexity=10);
}

module modified_shelf() {
    difference() {
        original_shelf();

        // RJ45 cutout (right wall, aligned with base)
        translate([cutout_wall_x + rj45_cutout_depth / 2, rj45_case_y, rj45_case_z])
            cube([rj45_cutout_depth, rj45_cutout_w, rj45_cutout_h], center=true);

        // Terminal cutout (right wall, aligned with base)
        translate([cutout_wall_x + terminal_cutout_depth / 2, terminal_case_y, terminal_case_z])
            cube([terminal_cutout_depth, terminal_cutout_w, terminal_cutout_h], center=true);
    }
}

modified_shelf();
