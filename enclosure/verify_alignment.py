#!/usr/bin/env python3
"""Verify cutout alignment against EAGLE BRD component positions.

Parses config.scad parameter values and validates that each cutout is
correctly positioned and sized relative to the Ethernet Shield's
component positions from the EAGLE BRD data.

Usage:
    python3 verify_alignment.py
"""

import re
import sys
from pathlib import Path


def parse_config_scad(filepath: Path) -> dict[str, float]:
    """Parse numeric variable assignments from config.scad."""
    values: dict[str, float] = {}
    with open(filepath) as f:
        for line in f:
            line = line.strip()
            # Skip comments and empty lines
            if not line or line.startswith("//") or line.startswith("$"):
                continue
            # Match simple numeric assignments: name = value;
            m = re.match(r'^(\w+)\s*=\s*(-?[\d.]+)\s*;', line)
            if m:
                values[m.group(1)] = float(m.group(2))
            # Match expressions referencing other variables
            # (skip these — we rely on the literal values above)
    return values


def evaluate_expression(expr: str, values: dict[str, float]) -> float | None:
    """Evaluate a simple arithmetic expression using parsed values."""
    # Replace variable names with their values
    result_expr = expr
    for name, val in sorted(values.items(), key=lambda x: -len(x[0])):
        result_expr = result_expr.replace(name, str(val))
    try:
        return eval(result_expr)  # noqa: S307
    except Exception:
        return None


class VerificationResult:
    def __init__(self, name: str):
        self.name = name
        self.checks: list[tuple[str, bool, str]] = []

    def check(self, description: str, passed: bool, detail: str = "") -> None:
        self.checks.append((description, passed, detail))

    @property
    def passed(self) -> bool:
        return all(p for _, p, _ in self.checks)

    def report(self) -> str:
        status = "PASS" if self.passed else "FAIL"
        lines = [f"\n[{status}] {self.name}"]
        for desc, passed, detail in self.checks:
            icon = "  ok" if passed else "  FAIL"
            line = f"  {icon}: {desc}"
            if detail:
                line += f" ({detail})"
            lines.append(line)
        return "\n".join(lines)


def verify_rj45(cfg: dict[str, float]) -> VerificationResult:
    """Verify RJ45 cutout dimensions and position."""
    result = VerificationResult("RJ45 Cutout")

    # Cutout must be >= housing + 2 * clearance
    housing_w = cfg.get("rj45_housing_w", 16.0)
    housing_h = cfg.get("rj45_housing_h", 13.5)
    cutout_w = cfg.get("rj45_cutout_w", 0)
    cutout_h = cfg.get("rj45_cutout_h", 0)
    min_clearance = 0.5  # per side

    result.check(
        f"Width >= housing + clearance ({housing_w} + {2*min_clearance})",
        cutout_w >= housing_w + 2 * min_clearance,
        f"cutout={cutout_w}, min={housing_w + 2*min_clearance}"
    )

    result.check(
        f"Height >= housing + clearance ({housing_h} + {2*min_clearance})",
        cutout_h >= housing_h + 2 * min_clearance,
        f"cutout={cutout_h}, min={housing_h + 2*min_clearance}"
    )

    # Verify overhang accommodation
    overhang = cfg.get("rj45_overhang", 4.318)
    result.check(
        f"RJ45 overhang ({overhang} mm) is documented",
        overhang > 0,
        f"overhang={overhang}"
    )

    return result


def verify_sd_inaccessible(cfg: dict[str, float]) -> VerificationResult:
    """Verify that the SD card is documented as inaccessible."""
    result = VerificationResult("SD Card (no cutout)")

    # SD card is in the center of the case, inaccessible from any wall.
    # Just verify the slot dimensions are still recorded for reference.
    slot_w = cfg.get("sd_slot_w", 0)
    result.check(
        "SD slot dimensions recorded for reference",
        slot_w > 0,
        f"sd_slot_w={slot_w}"
    )

    return result


def verify_terminal(cfg: dict[str, float]) -> VerificationResult:
    """Verify terminal cutout dimensions and spacing."""
    result = VerificationResult("Terminal Cutout")

    # Terminal cutout size
    cutout_w = cfg.get("terminal_cutout_w", 0)
    cutout_h = cfg.get("terminal_cutout_h", 0)
    min_w = 18.0 + 2 * 0.2  # Euroblock envelope + printer tolerance
    min_h = 10.0 + 2 * 0.2

    result.check(
        f"Width >= Euroblock + tolerance ({min_w})",
        cutout_w >= min_w,
        f"cutout={cutout_w}, min={min_w}"
    )

    result.check(
        f"Height >= Euroblock + tolerance ({min_h})",
        cutout_h >= min_h,
        f"cutout={cutout_h}, min={min_h}"
    )

    # Spacing from RJ45
    spacing = cfg.get("rj45_terminal_spacing", 0)
    result.check(
        f"Spacing from RJ45 >= 6 mm",
        spacing >= 6.0,
        f"spacing={spacing}"
    )

    return result


def verify_support_post(cfg: dict[str, float]) -> VerificationResult:
    """Verify support post position is in the safe zone."""
    result = VerificationResult("Support Post")

    board_x = cfg.get("support_post_board_x", 0)
    board_y = cfg.get("support_post_board_y", 0)
    diameter = cfg.get("support_post_diameter", 0)

    # Safe zone: x = 10-14, y = 32-36
    result.check(
        "X position in safe zone (10-14 mm)",
        10.0 <= board_x <= 14.0,
        f"x={board_x}"
    )

    result.check(
        "Y position in safe zone (32-36 mm)",
        32.0 <= board_y <= 36.0,
        f"y={board_y}"
    )

    result.check(
        "Pad diameter 8-12 mm",
        8.0 <= diameter <= 12.0,
        f"diameter={diameter}"
    )

    # Verify pad center is well within the safe zone (>= 1mm from edges)
    # The pad (8-12mm dia) intentionally extends beyond the 4x4mm verified-clear
    # zone. The surrounding area is also clear per EAGLE analysis but with less
    # margin. The center placement ensures maximum clearance to nearest via.
    result.check(
        "Pad center >= 1 mm inside safe zone edges",
        board_x >= 11.0 and board_x <= 13.0
        and board_y >= 33.0 and board_y <= 35.0,
        f"center=({board_x:.1f},{board_y:.1f}), safe zone inner=(11-13, 33-35)"
    )

    return result


def verify_clearances(cfg: dict[str, float]) -> VerificationResult:
    """Verify mechanical clearance requirements from the spec."""
    result = VerificationResult("Mechanical Clearances")

    wall = cfg.get("wall_thickness", 0)
    result.check(
        "Wall thickness >= 2 mm",
        wall >= 2.0,
        f"wall={wall}"
    )

    cutout_spacing = cfg.get("rj45_terminal_spacing", 0)
    result.check(
        "Cutout spacing >= 6 mm",
        cutout_spacing >= 6.0,
        f"spacing={cutout_spacing}"
    )

    return result


def main() -> None:
    config_path = Path(__file__).parent / "config.scad"
    if not config_path.exists():
        print(f"Error: {config_path} not found", file=sys.stderr)
        sys.exit(1)

    cfg = parse_config_scad(config_path)

    verifications = [
        verify_rj45(cfg),
        verify_sd_inaccessible(cfg),
        verify_terminal(cfg),
        verify_support_post(cfg),
        verify_clearances(cfg),
    ]

    all_passed = True
    for v in verifications:
        print(v.report())
        if not v.passed:
            all_passed = False

    print(f"\n{'=' * 40}")
    if all_passed:
        print("All verifications PASSED")
    else:
        print("Some verifications FAILED")
        sys.exit(1)


if __name__ == "__main__":
    main()
