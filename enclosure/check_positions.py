#!/usr/bin/env python3
"""Check board component positions in case coordinates.

Calculates where each component ends up relative to the case walls
and reports clearances and potential collisions.
"""

# Case wall positions (from analyze_stl.py)
base_x_min = -57.03
base_x_max =  57.87
base_y_min = -44.37
base_y_max =  44.53
base_z_min = -16.30
base_z_max =   3.86

wall_thickness = 4.0

# Inner wall positions
inner_x_min = base_x_min + wall_thickness  # -53.03
inner_x_max = base_x_max - wall_thickness  #  53.87
inner_y_min = base_y_min + wall_thickness  # -40.37
inner_y_max = base_y_max - wall_thickness  #  40.53

# Board-to-case transform
giga_board_origin_x = base_x_min + wall_thickness + 2.0  # -51.03
giga_board_origin_y = (base_y_min + base_y_max) / 2 - 76.2 / 2  # centered

eth_board_origin_x = giga_board_origin_x
eth_board_origin_y = giga_board_origin_y

# Stacking heights
standoff_height = 5.0
base_floor_z = base_z_min  # -16.30
giga_pcb_bottom_z = base_floor_z + standoff_height
giga_pcb_thickness = 1.6
giga_pcb_top_z = giga_pcb_bottom_z + giga_pcb_thickness
stacking_header_height = 8.5
eth_pcb_bottom_z = giga_pcb_top_z + stacking_header_height
eth_pcb_top_z = eth_pcb_bottom_z + 1.6

print("=" * 60)
print("Board Positions in Case Coordinates")
print("=" * 60)

print(f"\nCase inner walls:")
print(f"  X: {inner_x_min:.2f} to {inner_x_max:.2f} (width: {inner_x_max - inner_x_min:.2f})")
print(f"  Y: {inner_y_min:.2f} to {inner_y_max:.2f} (depth: {inner_y_max - inner_y_min:.2f})")
print(f"  Z: {base_z_min:.2f} to {base_z_max:.2f} (height: {base_z_max - base_z_min:.2f})")

print(f"\nGIGA R1 board origin (case coords): ({giga_board_origin_x:.2f}, {giga_board_origin_y:.2f})")
print(f"Ethernet Shield origin (case coords): ({eth_board_origin_x:.2f}, {eth_board_origin_y:.2f})")

print(f"\nEthernet Shield PCB extent:")
eth_x_min = eth_board_origin_x
eth_x_max = eth_board_origin_x + 68.6
eth_y_min = eth_board_origin_y
eth_y_max = eth_board_origin_y + 53.3
print(f"  X: {eth_x_min:.2f} to {eth_x_max:.2f}")
print(f"  Y: {eth_y_min:.2f} to {eth_y_max:.2f}")
print(f"  Z: {eth_pcb_bottom_z:.2f} to {eth_pcb_top_z:.2f}")

print(f"\n  Clearance to left wall (-X):  {eth_x_min - inner_x_min:.2f} mm")
print(f"  Clearance to right wall (+X): {inner_x_max - eth_x_max:.2f} mm")
print(f"  Clearance to front wall (-Y): {eth_y_min - inner_y_min:.2f} mm")
print(f"  Clearance to back wall (+Y):  {inner_y_max - eth_y_max:.2f} mm")

# RJ45 position
rj45_x = eth_board_origin_x + (-4.318)
rj45_y = eth_board_origin_y + 38.354
rj45_z = eth_pcb_top_z
print(f"\nRJ45 housing position (case coords):")
print(f"  Center: ({rj45_x:.2f}, {rj45_y:.2f})")
print(f"  Z base: {rj45_z:.2f}")
# RJ45 housing extends from board edge outward (-X direction)
rj45_housing_x_min = rj45_x - 21.75 + 4.318  # housing extends into -X
rj45_housing_x_max = rj45_x + 4.318
rj45_housing_y_min = rj45_y - 16.0 / 2
rj45_housing_y_max = rj45_y + 16.0 / 2
rj45_housing_z_min = rj45_z
rj45_housing_z_max = rj45_z + 13.5
print(f"  Housing X: {rj45_housing_x_min:.2f} to {rj45_housing_x_max:.2f}")
print(f"  Housing Y: {rj45_housing_y_min:.2f} to {rj45_housing_y_max:.2f}")
print(f"  Housing Z: {rj45_housing_z_min:.2f} to {rj45_housing_z_max:.2f}")
print(f"  Distance to left wall (-X): {rj45_housing_x_min - base_x_min:.2f} mm (negative = through wall)")

# Cutout position
rj45_cutout_y = eth_board_origin_y + 38.354
rj45_cutout_z = eth_pcb_top_z + 13.5 / 2
print(f"\nRJ45 cutout position:")
print(f"  Center Y: {rj45_cutout_y:.2f} (housing center Y: {rj45_y:.2f}) delta: {rj45_cutout_y - rj45_y:.2f}")
print(f"  Center Z: {rj45_cutout_z:.2f} (housing center Z: {(rj45_housing_z_min + rj45_housing_z_max)/2:.2f}) delta: {rj45_cutout_z - (rj45_housing_z_min + rj45_housing_z_max)/2:.2f}")

# SD position
sd_x = eth_board_origin_x + 63.5
sd_y = eth_board_origin_y + 19.05
sd_z = eth_pcb_top_z
print(f"\nSD slot position (case coords):")
print(f"  Center: ({sd_x:.2f}, {sd_y:.2f})")
print(f"  Z base: {sd_z:.2f}")
sd_housing_x_min = sd_x - 14.0 / 2
sd_housing_x_max = sd_x + 14.0 / 2
sd_housing_y_min = sd_y - 12.0 / 2
sd_housing_y_max = sd_y + 12.0 / 2
print(f"  Housing X: {sd_housing_x_min:.2f} to {sd_housing_x_max:.2f}")
print(f"  Housing Y: {sd_housing_y_min:.2f} to {sd_housing_y_max:.2f}")
print(f"  Distance to right wall (+X): {base_x_max - sd_housing_x_max:.2f} mm (negative = through wall)")

# Terminal cutout
terminal_y = rj45_cutout_y + 17.0/2 + 6.0 + 18.4/2
terminal_z = rj45_cutout_z
print(f"\nTerminal cutout position:")
print(f"  Center Y: {terminal_y:.2f}")
print(f"  Center Z: {terminal_z:.2f}")
print(f"  Distance to back wall (+Y): {inner_y_max - (terminal_y + 18.4/2):.2f} mm")
print(f"  Distance to case edge (+Y): {base_y_max - (terminal_y + 18.4/2):.2f} mm")

# Check if RJ45 housing reaches the wall
print(f"\n{'=' * 60}")
print("COLLISION CHECK")
print(f"{'=' * 60}")

issues = []

# RJ45 vs left wall
if rj45_housing_x_min < inner_x_min:
    if rj45_housing_x_min < base_x_min:
        issues.append(f"RJ45 housing extends {base_x_min - rj45_housing_x_min:.2f} mm PAST the outer wall")
    else:
        print(f"  ok: RJ45 housing penetrates wall (expected - cutout provides clearance)")
else:
    print(f"  WARNING: RJ45 housing does NOT reach wall (gap: {rj45_housing_x_min - inner_x_min:.2f} mm)")

# SD vs right wall
if sd_housing_x_max > inner_x_max:
    if sd_housing_x_max > base_x_max:
        issues.append(f"SD housing extends {sd_housing_x_max - base_x_max:.2f} mm PAST the outer wall")
    else:
        print(f"  ok: SD slot reaches into wall (expected - cutout provides clearance)")
else:
    print(f"  WARNING: SD slot does NOT reach wall (gap: {inner_x_max - sd_housing_x_max:.2f} mm)")

# PCB vs walls
if eth_x_min < inner_x_min:
    issues.append(f"PCB left edge clips inner wall by {inner_x_min - eth_x_min:.2f} mm")
if eth_x_max > inner_x_max:
    issues.append(f"PCB right edge clips inner wall by {eth_x_max - inner_x_max:.2f} mm")
if eth_y_min < inner_y_min:
    issues.append(f"PCB front edge clips inner wall by {inner_y_min - eth_y_min:.2f} mm")
if eth_y_max > inner_y_max:
    issues.append(f"PCB back edge clips inner wall by {eth_y_max - inner_y_max:.2f} mm")

if issues:
    print("\nISSUES FOUND:")
    for issue in issues:
        print(f"  ** {issue}")
else:
    print("\n  No collisions detected.")
