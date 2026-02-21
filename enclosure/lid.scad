// lid.scad — Modified lid with display window filled solid
//
// Takes the original Display Shield lid and fills the display window
// opening with a solid rectangular block, creating a sealed top surface.

include <config.scad>

module original_lid() {
    import(original_lid, convexity=10);
}

module window_fill_block() {
    // Rectangular block that fills the display window opening.
    // Slightly oversized in X/Y to overlap with existing wall material
    // (the union takes care of the overlap). Thickness spans from the
    // inside surface up to the outer surface.

    fill_x = (window_x_max - window_x_min) + 2 * window_fill_margin;
    fill_y = (window_y_max - window_y_min) + 2 * window_fill_margin;
    fill_z = window_fill_thickness;

    center_x = (window_x_min + window_x_max) / 2;
    center_y = (window_y_min + window_y_max) / 2;

    translate([center_x, center_y, window_z - fill_z])
        cube([fill_x, fill_y, fill_z], center=true);
}

module modified_lid() {
    union() {
        original_lid();
        window_fill_block();
    }
}

// Render the modified lid
modified_lid();
