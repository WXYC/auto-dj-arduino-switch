// test_slices.scad — Thin wall slices for pre-print fit testing
//
// Extracts 5mm-thick slices from the modified case at the right wall
// where the RJ45 and terminal cutouts are.
//
// Usage:
//   openscad -D 'slice="rj45"' -o output/test_slice_rj45_wall.stl test_slices.scad

include <config.scad>

slice = "rj45";  // "rj45" (includes terminal on same wall)

slice_thickness = 5.0;  // mm

module rj45_wall_slice() {
    // Slice of the right wall (+X) containing RJ45 and terminal cutouts
    intersection() {
        union() {
            difference() {
                import(original_base, convexity=10);
                // RJ45 cutout
                translate([cutout_wall_x + 5, rj45_case_y, rj45_case_z])
                    cube([10, rj45_cutout_w, rj45_cutout_h], center=true);
                // Terminal cutout
                translate([cutout_wall_x + 5, terminal_case_y, terminal_case_z])
                    cube([10, terminal_cutout_w, terminal_cutout_h], center=true);
            }
            difference() {
                import(original_shelf, convexity=10);
                translate([cutout_wall_x + 5, rj45_case_y, rj45_case_z])
                    cube([10, rj45_cutout_w, rj45_cutout_h], center=true);
                translate([cutout_wall_x + 5, terminal_case_y, terminal_case_z])
                    cube([10, terminal_cutout_w, terminal_cutout_h], center=true);
            }
        }

        // Extraction box centered on the right wall
        translate([cutout_wall_x, 0, 0])
            cube([slice_thickness, 200, 200], center=true);
    }
}

if (slice == "rj45") {
    rj45_wall_slice();
} else {
    echo("ERROR: Unknown slice. Use slice=\"rj45\"");
}
