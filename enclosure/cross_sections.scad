// cross_sections.scad — 2D cross-section projections for paper templates
//
// Generates 2D slices at key heights that can be exported as SVG,
// printed at 1:1 scale on paper, and used for zero-cost fit checks
// against the physical boards.
//
// Usage:
//   openscad -D 'section="rj45"' -o output/cross_section_rj45.svg cross_sections.scad
//   openscad -D 'section="sd"' -o output/cross_section_sd.svg cross_sections.scad
//   openscad -D 'section="terminal"' -o output/cross_section_terminal.svg cross_sections.scad

include <config.scad>

// Which cross-section to render (set via -D on command line)
section = "rj45";  // "rj45" | "sd" | "terminal"

module case_assembly() {
    // Combined base + shelf for cross-sectioning
    union() {
        difference() {
            import(original_base, convexity=10);
            // RJ45 cutout
            translate([rj45_wall_x - 5, rj45_case_y, rj45_case_z])
                cube([10, rj45_cutout_w, rj45_cutout_h], center=true);
            // SD cutout
            translate([sd_wall_x + 5, sd_case_y, sd_case_z])
                cube([10, sd_cutout_w, sd_cutout_h], center=true);
            // Terminal cutout
            translate([rj45_wall_x - 5, terminal_case_y, terminal_case_z])
                cube([10, terminal_cutout_w, terminal_cutout_h], center=true);
        }
        difference() {
            import(original_shelf, convexity=10);
            translate([rj45_wall_x - 5, rj45_case_y, rj45_case_z])
                cube([10, rj45_cutout_w, rj45_cutout_h], center=true);
            translate([sd_wall_x + 5, sd_case_y, sd_case_z])
                cube([10, sd_cutout_w, sd_cutout_h], center=true);
            translate([rj45_wall_x - 5, terminal_case_y, terminal_case_z])
                cube([10, terminal_cutout_w, terminal_cutout_h], center=true);
        }
    }
}

module rj45_cross_section() {
    // Horizontal slice at RJ45 center height
    projection(cut=true)
        translate([0, 0, -rj45_case_z])
            case_assembly();
}

module sd_cross_section() {
    // Horizontal slice at SD slot center height
    projection(cut=true)
        translate([0, 0, -sd_case_z])
            case_assembly();
}

module terminal_cross_section() {
    // Horizontal slice at terminal center height
    projection(cut=true)
        translate([0, 0, -terminal_case_z])
            case_assembly();
}

// Render the selected cross-section
if (section == "rj45") {
    rj45_cross_section();
} else if (section == "sd") {
    sd_cross_section();
} else if (section == "terminal") {
    terminal_cross_section();
} else {
    echo("ERROR: Unknown section. Use section=\"rj45\", \"sd\", or \"terminal\"");
}
