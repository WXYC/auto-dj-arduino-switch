// test_slices.scad — Thin wall slices for pre-print fit testing
//
// Extracts 5mm-thick slices from the modified case at each cutout wall
// position. Print these individually for quick (~3-5 min) test fits
// before committing to a full case print.
//
// Usage:
//   openscad -D 'slice="rj45"' -o output/test_slice_rj45_wall.stl test_slices.scad
//   openscad -D 'slice="sd"' -o output/test_slice_sd_wall.stl test_slices.scad

include <config.scad>

// Which slice to render (set via -D on command line)
slice = "rj45";  // "rj45" | "sd"

slice_thickness = 5.0;  // mm

module rj45_wall_slice() {
    // Slice of the left wall (-X) containing RJ45 and terminal cutouts
    intersection() {
        union() {
            // Import both base and shelf (they share this wall)
            difference() {
                import(original_base, convexity=10);
                // RJ45 cutout
                translate([rj45_wall_x - 5, rj45_case_y, rj45_case_z])
                    cube([10, rj45_cutout_w, rj45_cutout_h], center=true);
                // Terminal cutout
                translate([rj45_wall_x - 5, terminal_case_y, terminal_case_z])
                    cube([10, terminal_cutout_w, terminal_cutout_h], center=true);
            }
            difference() {
                import(original_shelf, convexity=10);
                translate([rj45_wall_x - 5, rj45_case_y, rj45_case_z])
                    cube([10, rj45_cutout_w, rj45_cutout_h], center=true);
                translate([rj45_wall_x - 5, terminal_case_y, terminal_case_z])
                    cube([10, terminal_cutout_w, terminal_cutout_h], center=true);
            }
        }

        // Extraction box: slice centered on the left wall
        translate([rj45_wall_x, 0, 0])
            cube([slice_thickness, 200, 200], center=true);
    }
}

module sd_wall_slice() {
    // Slice of the right wall (+X) containing SD card cutout
    intersection() {
        union() {
            difference() {
                import(original_base, convexity=10);
                translate([sd_wall_x + 5, sd_case_y, sd_case_z])
                    cube([10, sd_cutout_w, sd_cutout_h], center=true);
                // SD chamfer
                translate([sd_wall_x + 1, sd_case_y, sd_case_z - sd_cutout_h/2 - sd_chamfer_height/2])
                    hull() {
                        cube([sd_chamfer_depth, sd_cutout_w, 0.01], center=true);
                        translate([0, 0, -sd_chamfer_height])
                            cube([0.01, sd_cutout_w - 2, 0.01], center=true);
                    }
            }
            difference() {
                import(original_shelf, convexity=10);
                translate([sd_wall_x + 5, sd_case_y, sd_case_z])
                    cube([10, sd_cutout_w, sd_cutout_h], center=true);
            }
        }

        // Extraction box: slice centered on the right wall
        translate([sd_wall_x, 0, 0])
            cube([slice_thickness, 200, 200], center=true);
    }
}

// Render the selected slice
if (slice == "rj45") {
    rj45_wall_slice();
} else if (slice == "sd") {
    sd_wall_slice();
} else {
    echo("ERROR: Unknown slice type. Use slice=\"rj45\" or slice=\"sd\"");
}
