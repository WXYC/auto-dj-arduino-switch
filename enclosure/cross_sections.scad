// cross_sections.scad — 2D cross-section projections for paper templates
//
// Usage:
//   openscad -D 'section="rj45"' -o output/cross_section_rj45.svg cross_sections.scad
//   openscad -D 'section="terminal"' -o output/cross_section_terminal.svg cross_sections.scad

include <config.scad>

section = "rj45";  // "rj45" | "terminal"

module case_assembly() {
    union() {
        difference() {
            import(original_base, convexity=10);
            translate([cutout_wall_x + 5, rj45_case_y, rj45_case_z])
                cube([10, rj45_cutout_w, rj45_cutout_h], center=true);
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
}

module rj45_cross_section() {
    projection(cut=true)
        translate([0, 0, -rj45_case_z])
            case_assembly();
}

module terminal_cross_section() {
    projection(cut=true)
        translate([0, 0, -terminal_case_z])
            case_assembly();
}

if (section == "rj45") {
    rj45_cross_section();
} else if (section == "terminal") {
    terminal_cross_section();
} else {
    echo("ERROR: Unknown section. Use section=\"rj45\" or \"terminal\"");
}
