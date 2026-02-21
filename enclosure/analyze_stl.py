#!/usr/bin/env python3
"""Analyze binary STL files and report bounding boxes and key dimensions.

Parses binary STL format (no external dependencies) and prints per-file
bounding box, center, and dimensions. Used to extract geometry needed
for OpenSCAD modification parameters (display window bounds, wall
positions, etc.).

Usage:
    python3 analyze_stl.py [stl_file ...]
    python3 analyze_stl.py          # analyzes all files in original/
"""

import struct
import sys
from pathlib import Path


def parse_binary_stl(filepath: Path) -> list[tuple[tuple[float, float, float], ...]]:
    """Parse a binary STL file and return a list of triangles.

    Each triangle is a tuple of three (x, y, z) vertices.
    """
    triangles = []
    with open(filepath, "rb") as f:
        header = f.read(80)  # noqa: F841
        (num_triangles,) = struct.unpack("<I", f.read(4))

        for _ in range(num_triangles):
            # Normal vector (12 bytes) - skip
            f.read(12)
            # Three vertices, each 3 floats (36 bytes total)
            v1 = struct.unpack("<fff", f.read(12))
            v2 = struct.unpack("<fff", f.read(12))
            v3 = struct.unpack("<fff", f.read(12))
            # Attribute byte count (2 bytes)
            f.read(2)
            triangles.append((v1, v2, v3))

    return triangles


def bounding_box(
    triangles: list[tuple[tuple[float, float, float], ...]],
) -> dict:
    """Compute the axis-aligned bounding box from a list of triangles."""
    xs, ys, zs = [], [], []
    for tri in triangles:
        for x, y, z in tri:
            xs.append(x)
            ys.append(y)
            zs.append(z)

    return {
        "min": (min(xs), min(ys), min(zs)),
        "max": (max(xs), max(ys), max(zs)),
        "size": (max(xs) - min(xs), max(ys) - min(ys), max(zs) - min(zs)),
        "center": (
            (min(xs) + max(xs)) / 2,
            (min(ys) + max(ys)) / 2,
            (min(zs) + max(zs)) / 2,
        ),
    }


def find_z_layers(
    triangles: list[tuple[tuple[float, float, float], ...]],
    tolerance: float = 0.1,
) -> list[float]:
    """Find distinct Z heights where many vertices cluster (wall/floor levels)."""
    z_values: dict[float, int] = {}
    for tri in triangles:
        for _, _, z in tri:
            rounded = round(z / tolerance) * tolerance
            z_values[rounded] = z_values.get(rounded, 0) + 1

    # Return Z heights with significant vertex counts, sorted
    threshold = len(triangles) * 0.01  # at least 1% of triangles
    layers = sorted(z for z, count in z_values.items() if count >= threshold)
    return layers


def find_openings(
    triangles: list[tuple[tuple[float, float, float], ...]],
    bbox: dict,
    axis: str = "z",
    level: float | None = None,
) -> list[dict]:
    """Identify rectangular openings on a given face by finding gaps in
    the mesh at a particular level.

    This is a simplified analysis that looks for the bounding box of
    vertices at a specific Z level (for the top face) to identify the
    display window region.
    """
    if axis != "z":
        return []  # Only Z-axis (top face) analysis implemented

    if level is None:
        level = bbox["max"][2]

    tolerance = 0.5  # mm
    # Collect all vertices near the top Z level
    top_verts = []
    for tri in triangles:
        for x, y, z in tri:
            if abs(z - level) < tolerance:
                top_verts.append((x, y))

    if not top_verts:
        return []

    # Find the interior bounds (where vertices exist on the top face)
    xs = [v[0] for v in top_verts]
    ys = [v[1] for v in top_verts]

    # The display window is the region on the top face. We detect it by
    # looking for a gap in the X-Y vertex coverage.
    # For a simple analysis, report the top-face vertex extent.
    return [
        {
            "face": "top",
            "x_range": (min(xs), max(xs)),
            "y_range": (min(ys), max(ys)),
            "z": level,
        }
    ]


def analyze_file(filepath: Path) -> None:
    """Analyze a single STL file and print results."""
    print(f"\n{'=' * 60}")
    print(f"File: {filepath.name}")
    print(f"{'=' * 60}")

    triangles = parse_binary_stl(filepath)
    bbox = bounding_box(triangles)

    print(f"  Triangles: {len(triangles)}")
    print(f"  Bounding box:")
    print(f"    Min: ({bbox['min'][0]:.2f}, {bbox['min'][1]:.2f}, {bbox['min'][2]:.2f})")
    print(f"    Max: ({bbox['max'][0]:.2f}, {bbox['max'][1]:.2f}, {bbox['max'][2]:.2f})")
    print(f"  Size: {bbox['size'][0]:.2f} x {bbox['size'][1]:.2f} x {bbox['size'][2]:.2f} mm")
    print(f"  Center: ({bbox['center'][0]:.2f}, {bbox['center'][1]:.2f}, {bbox['center'][2]:.2f})")

    layers = find_z_layers(triangles)
    if layers:
        print(f"  Key Z layers: {', '.join(f'{z:.2f}' for z in layers[:10])}")

    # For the lid, try to identify the display window
    if "lid" in filepath.name.lower() or "top" in filepath.name.lower():
        openings = find_openings(triangles, bbox)
        if openings:
            print(f"  Top face analysis:")
            for opening in openings:
                print(
                    f"    X range: {opening['x_range'][0]:.2f} to {opening['x_range'][1]:.2f}"
                )
                print(
                    f"    Y range: {opening['y_range'][0]:.2f} to {opening['y_range'][1]:.2f}"
                )


def main() -> None:
    if len(sys.argv) > 1:
        files = [Path(f) for f in sys.argv[1:]]
    else:
        original_dir = Path(__file__).parent / "original"
        files = sorted(original_dir.glob("*.stl"))

    if not files:
        print("No STL files found.", file=sys.stderr)
        sys.exit(1)

    for filepath in files:
        if not filepath.exists():
            print(f"File not found: {filepath}", file=sys.stderr)
            continue
        analyze_file(filepath)


if __name__ == "__main__":
    main()
