#!/usr/bin/env python3
"""Generate cross-section SVGs by slicing rendered STL meshes at key heights.

Replaces the OpenSCAD projection(cut=true) approach, which fails on complex
meshes due to CGAL Nef polyhedron limitations in OpenSCAD 2021.01.

Slices the already-rendered output STLs (base.stl + shelf.stl) that have all
modifications (cutouts, fill blocks) baked in. No external dependencies.

Usage:
    python3 cross_section.py rj45       # output/cross_section_rj45.svg
    python3 cross_section.py sd         # output/cross_section_sd.svg
    python3 cross_section.py terminal   # output/cross_section_terminal.svg
"""

import re
import sys
from pathlib import Path

from analyze_stl import parse_binary_stl

# Type alias for a triangle: three (x, y, z) vertices.
Triangle = tuple[tuple[float, float, float], ...]

# Map CLI section names to the config.scad variables needed to compute
# the effective slice height.  The cutout center Z may be above the case
# wall, so we intersect the cutout range with the wall Z range and slice
# at the midpoint of the overlap.
SECTIONS = {
    "rj45": {"center_z": "rj45_case_z", "cutout_h": "rj45_cutout_h"},
    "sd": {"center_z": "sd_case_z", "cutout_h": "sd_cutout_h"},
    "terminal": {"center_z": "terminal_case_z", "cutout_h": "terminal_cutout_h"},
}


def parse_ascii_stl(filepath: Path) -> list[Triangle]:
    """Parse an ASCII STL file and return a list of triangles."""
    triangles: list[Triangle] = []
    vertices: list[tuple[float, float, float]] = []

    with open(filepath) as f:
        for line in f:
            line = line.strip()
            if line.startswith("vertex"):
                parts = line.split()
                vertices.append((float(parts[1]), float(parts[2]), float(parts[3])))
                if len(vertices) == 3:
                    triangles.append(tuple(vertices))  # type: ignore[arg-type]
                    vertices = []

    return triangles


def parse_stl(filepath: Path) -> list[Triangle]:
    """Parse an STL file, auto-detecting ASCII vs binary format."""
    with open(filepath, "rb") as f:
        header = f.read(80)

    if header.startswith(b"solid") and b"\n" in header:
        # Likely ASCII (binary files can also start with "solid" but
        # won't have a newline within the 80-byte header).
        return parse_ascii_stl(filepath)

    return parse_binary_stl(filepath)


def parse_config_values(filepath: Path) -> dict[str, float]:
    """Parse variable assignments from config.scad, evaluating expressions.

    First pass: collect literal numeric values.
    Subsequent passes: substitute known values into expressions and evaluate
    until no more can be resolved.
    """
    literals: dict[str, float] = {}
    expressions: dict[str, str] = {}

    with open(filepath) as f:
        for line in f:
            line = line.strip()
            if not line or line.startswith("//") or line.startswith("$"):
                continue
            m = re.match(r"^(\w+)\s*=\s*(.+?)\s*;", line)
            if not m:
                continue
            name = m.group(1)
            expr = m.group(2).strip()
            # Try as a literal number first
            try:
                literals[name] = float(expr)
            except ValueError:
                # Skip array-like values (contain [ ])
                if "[" not in expr:
                    expressions[name] = expr

    # Iteratively resolve expressions that reference already-known values.
    values = dict(literals)
    changed = True
    while changed:
        changed = False
        for name, expr in list(expressions.items()):
            if name in values:
                continue
            # Substitute known variable names (longest first to avoid
            # partial matches, e.g. "base_z_min" before "base_z").
            resolved = expr
            for var, val in sorted(values.items(), key=lambda x: -len(x[0])):
                resolved = resolved.replace(var, repr(val))
            try:
                result = eval(resolved)  # noqa: S307
                if isinstance(result, (int, float)):
                    values[name] = float(result)
                    changed = True
            except Exception:
                pass

    return values


def slice_triangles(
    triangles: list[tuple[tuple[float, float, float], ...]],
    z: float,
) -> list[tuple[tuple[float, float], tuple[float, float]]]:
    """Compute the intersection of triangles with a horizontal plane at z.

    For each triangle edge, if the two vertices straddle the plane, linearly
    interpolate to find the (x, y) intersection point. Two such points per
    intersected triangle form one line segment.

    Returns a list of line segments as ((x1, y1), (x2, y2)).
    """
    segments = []
    eps = 1e-9

    for tri in triangles:
        points: list[tuple[float, float]] = []

        for i in range(3):
            v1 = tri[i]
            v2 = tri[(i + 1) % 3]
            z1, z2 = v1[2], v2[2]
            dz = z2 - z1

            if abs(dz) < eps:
                # Edge is parallel to the slice plane.
                if abs(z1 - z) < eps:
                    # Edge lies in the plane: contribute both endpoints.
                    points.append((v1[0], v1[1]))
                    points.append((v2[0], v2[1]))
                continue

            t = (z - z1) / dz
            if -eps <= t <= 1.0 + eps:
                t = max(0.0, min(1.0, t))
                x = v1[0] + t * (v2[0] - v1[0])
                y = v1[1] + t * (v2[1] - v1[1])
                points.append((x, y))

        # Deduplicate points that are very close together.
        unique: list[tuple[float, float]] = []
        for p in points:
            if not any(
                abs(p[0] - q[0]) < eps and abs(p[1] - q[1]) < eps for q in unique
            ):
                unique.append(p)

        if len(unique) == 2:
            segments.append((unique[0], unique[1]))

    return segments


def write_svg(
    segments: list[tuple[tuple[float, float], tuple[float, float]]],
    output_path: Path,
    section_name: str,
) -> None:
    """Write line segments to an SVG file at 1:1 scale (units = mm)."""
    if not segments:
        print(
            f"Warning: no intersection segments for '{section_name}'", file=sys.stderr
        )
        output_path.write_text(
            '<?xml version="1.0" encoding="UTF-8"?>\n'
            '<svg xmlns="http://www.w3.org/2000/svg" width="1mm" height="1mm" '
            'viewBox="0 0 1 1"></svg>\n'
        )
        return

    all_x = [p[0] for seg in segments for p in seg]
    all_y = [p[1] for seg in segments for p in seg]
    x_min, x_max = min(all_x), max(all_x)
    y_min, y_max = min(all_y), max(all_y)

    margin = 2.0  # mm padding around the drawing
    vb_x = x_min - margin
    vb_y = y_min - margin
    vb_w = (x_max - x_min) + 2 * margin
    vb_h = (y_max - y_min) + 2 * margin

    lines = [
        '<?xml version="1.0" encoding="UTF-8"?>',
        f'<svg xmlns="http://www.w3.org/2000/svg"',
        f'     width="{vb_w:.2f}mm" height="{vb_h:.2f}mm"',
        f'     viewBox="{vb_x:.4f} {vb_y:.4f} {vb_w:.4f} {vb_h:.4f}">',
        f"  <!-- Cross-section: {section_name} -->",
        f"  <!-- {len(segments)} line segments -->",
        '  <g stroke="black" stroke-width="0.2" stroke-linecap="round" fill="none">',
    ]

    for (x1, y1), (x2, y2) in segments:
        lines.append(
            f'    <line x1="{x1:.4f}" y1="{y1:.4f}" x2="{x2:.4f}" y2="{y2:.4f}"/>'
        )

    lines.append("  </g>")
    lines.append("</svg>")
    lines.append("")

    output_path.write_text("\n".join(lines))


def main() -> None:
    if len(sys.argv) != 2 or sys.argv[1] not in SECTIONS:
        print(
            f"Usage: {sys.argv[0]} <{'|'.join(SECTIONS)}>",
            file=sys.stderr,
        )
        sys.exit(1)

    section = sys.argv[1]
    script_dir = Path(__file__).parent
    config_path = script_dir / "config.scad"
    output_dir = script_dir / "output"
    base_stl = output_dir / "base.stl"
    shelf_stl = output_dir / "shelf.stl"

    # Parse config to compute the effective cross-section Z height.
    values = parse_config_values(config_path)
    sec = SECTIONS[section]
    for key in (sec["center_z"], sec["cutout_h"], "base_z_min", "base_z_max"):
        if key not in values:
            print(f"Error: could not resolve '{key}' from config.scad", file=sys.stderr)
            sys.exit(1)

    # The cutout center Z can be above the case wall (e.g. RJ45 housing
    # center at 7.15 mm vs wall top at 3.86 mm).  Compute where the
    # cutout actually intersects the wall and slice at the midpoint.
    center_z = values[sec["center_z"]]
    cutout_h = values[sec["cutout_h"]]
    wall_z_min = values["base_z_min"]
    wall_z_max = values["base_z_max"]

    cutout_bot = center_z - cutout_h / 2
    cutout_top = center_z + cutout_h / 2
    effective_bot = max(cutout_bot, wall_z_min)
    effective_top = min(cutout_top, wall_z_max)

    if effective_bot >= effective_top:
        print(
            f"Error: cutout [{cutout_bot:.2f}, {cutout_top:.2f}] does not overlap "
            f"wall [{wall_z_min:.2f}, {wall_z_max:.2f}]",
            file=sys.stderr,
        )
        sys.exit(1)

    z = (effective_bot + effective_top) / 2
    print(f"Cross-section '{section}' at Z = {z:.2f} mm")
    print(f"  (cutout center={center_z:.2f}, wall=[{wall_z_min:.2f}, {wall_z_max:.2f}])")

    # Load and combine triangles from both rendered STLs.
    triangles: list[tuple[tuple[float, float, float], ...]] = []
    for stl_path in (base_stl, shelf_stl):
        if not stl_path.exists():
            print(
                f"Error: {stl_path} not found (run 'make stls' first)",
                file=sys.stderr,
            )
            sys.exit(1)
        tris = parse_stl(stl_path)
        triangles.extend(tris)
        print(f"  Loaded {len(tris)} triangles from {stl_path.name}")

    # Slice the combined mesh.
    segments = slice_triangles(triangles, z)
    print(f"  {len(segments)} intersection segments")

    # Write SVG.
    output_path = output_dir / f"cross_section_{section}.svg"
    write_svg(segments, output_path, section)
    print(f"  Wrote {output_path}")


if __name__ == "__main__":
    main()
