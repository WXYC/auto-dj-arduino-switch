#!/usr/bin/env python3
"""Find mounting standoff positions in the base STL.

Standoffs are cylindrical features rising from the base floor. This
script looks for clusters of vertices at specific Z ranges that form
circular patterns, indicating standoff positions.
"""

import math
import struct
from collections import defaultdict
from pathlib import Path


def parse_binary_stl(filepath: Path) -> list[tuple[tuple[float, ...], ...]]:
    triangles = []
    with open(filepath, "rb") as f:
        f.read(80)
        (num_triangles,) = struct.unpack("<I", f.read(4))
        for _ in range(num_triangles):
            f.read(12)
            v1 = struct.unpack("<fff", f.read(12))
            v2 = struct.unpack("<fff", f.read(12))
            v3 = struct.unpack("<fff", f.read(12))
            f.read(2)
            triangles.append((v1, v2, v3))
    return triangles


def main() -> None:
    base_path = Path(__file__).parent / "original" / "base.stl"
    triangles = parse_binary_stl(base_path)

    # The base floor is at Z = -16.30.
    # Standoffs rise from the floor. They'd be cylindrical features
    # with vertices at various Z levels between the floor and the
    # standoff top.
    #
    # Strategy: Find all vertices in a Z band above the floor
    # (standoff height region), then cluster them by (x,y) to find
    # circular patterns.

    base_floor_z = -16.30
    # Standoffs are typically 3-8mm tall
    standoff_z_min = base_floor_z + 1.0
    standoff_z_max = base_floor_z + 12.0

    # Collect vertices in the standoff Z range
    standoff_verts = []
    for tri in triangles:
        for x, y, z in tri:
            if standoff_z_min < z < standoff_z_max:
                standoff_verts.append((x, y, z))

    print(f"Vertices in standoff Z range ({standoff_z_min:.1f} to {standoff_z_max:.1f}): {len(standoff_verts)}")

    # Grid the X-Y plane and count vertices per cell
    grid_size = 1.0  # mm
    grid: dict[tuple[int, int], list[tuple[float, float, float]]] = defaultdict(list)
    for x, y, z in standoff_verts:
        gx = round(x / grid_size)
        gy = round(y / grid_size)
        grid[(gx, gy)].append((x, y, z))

    # Find cells with high vertex density (standoff locations)
    # Standoffs are small cylinders (~3-6mm diameter) so they'll have
    # many vertices concentrated in a small area
    dense_cells = [(k, v) for k, v in grid.items() if len(v) >= 5]
    dense_cells.sort(key=lambda x: -len(x[1]))

    if not dense_cells:
        print("No dense vertex clusters found in standoff Z range")
        return

    # Cluster nearby dense cells into standoff positions
    standoff_centers = []
    used = set()

    for (gx, gy), verts in dense_cells:
        if (gx, gy) in used:
            continue

        # Flood-fill to find all adjacent dense cells
        cluster_verts = []
        queue = [(gx, gy)]
        while queue:
            cx, cy = queue.pop()
            if (cx, cy) in used:
                continue
            cell_verts = grid.get((cx, cy), [])
            if len(cell_verts) < 3:
                continue
            used.add((cx, cy))
            cluster_verts.extend(cell_verts)
            for dx in [-1, 0, 1]:
                for dy in [-1, 0, 1]:
                    if (cx + dx, cy + dy) not in used:
                        queue.append((cx + dx, cy + dy))

        if len(cluster_verts) >= 20:
            # Compute center and Z range
            xs = [v[0] for v in cluster_verts]
            ys = [v[1] for v in cluster_verts]
            zs = [v[2] for v in cluster_verts]
            cx = sum(xs) / len(xs)
            cy = sum(ys) / len(ys)

            # Estimate diameter from XY spread
            max_dist = max(
                math.sqrt((x - cx) ** 2 + (y - cy) ** 2)
                for x, y, _ in cluster_verts
            )

            standoff_centers.append({
                "x": cx,
                "y": cy,
                "z_min": min(zs),
                "z_max": max(zs),
                "diameter": max_dist * 2,
                "vertex_count": len(cluster_verts),
            })

    # Sort by X then Y
    standoff_centers.sort(key=lambda s: (s["x"], s["y"]))

    print(f"\nFound {len(standoff_centers)} potential standoff features:\n")
    for i, s in enumerate(standoff_centers):
        print(f"  #{i+1}: center=({s['x']:.2f}, {s['y']:.2f}), "
              f"Z={s['z_min']:.2f} to {s['z_max']:.2f}, "
              f"dia~{s['diameter']:.1f}mm, "
              f"verts={s['vertex_count']}")

    # Filter to likely mounting standoffs (small diameter, consistent height)
    print("\nLikely mounting standoffs (diameter 4-12mm):")
    mounting = [s for s in standoff_centers if 4.0 <= s["diameter"] <= 12.0]
    for i, s in enumerate(mounting):
        height = s["z_max"] - base_floor_z
        print(f"  #{i+1}: ({s['x']:.2f}, {s['y']:.2f}), "
              f"height~{height:.1f}mm, dia~{s['diameter']:.1f}mm")

    # Also look at the right wall opening to determine USB-C position
    print("\n\nRight wall (+X) opening analysis:")
    right_wall_x = 57.87
    tol = 1.0
    right_wall_verts = []
    for tri in triangles:
        for x, y, z in tri:
            if abs(x - right_wall_x) < tol:
                right_wall_verts.append((y, z))

    if right_wall_verts:
        ys = [v[0] for v in right_wall_verts]
        zs = [v[1] for v in right_wall_verts]
        print(f"  Y range: {min(ys):.2f} to {max(ys):.2f}")
        print(f"  Z range: {min(zs):.2f} to {max(zs):.2f}")


if __name__ == "__main__":
    main()
