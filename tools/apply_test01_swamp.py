#!/usr/bin/env python3
"""Build the Mireglen Swamp starting map.

This intentionally rebuilds the outdoor terrain as one coherent composition
instead of sprinkling swamp tiles over Littleroot.  Existing house graphics are
retained, Birch's lab is moved to the central hummock, and a continuous marsh
with proper shoreline metatiles surrounds the settlement.
"""
from pathlib import Path
import struct

ROOT = Path(__file__).resolve().parents[1]
MAP = ROOT / "data" / "layouts" / "LittlerootTown" / "map.bin"
WIDTH = 20
HEIGHT = 20
METATILE_MASK = 0x03FF

# General/Petalburg metatiles used by this layout.
GRASS = 0x001
FLOWER = 0x004
TALL_GRASS = 0x00D
LONG_GRASS = 0x015
REFLECTIVE_WATER = 0x0A1
WATER_TOP_LEFT = 0x0B0
WATER_TOP = 0x0B1
WATER_TOP_RIGHT = 0x0B2
WATER_LEFT_EDGE = 0x0B8      # land west, water east
WATER_RIGHT_EDGE = 0x0BA     # water west, land east
WATER_INNER_LEFT = 0x0C0
WATER_INNER_RIGHT = 0x0C2
SIGN = 0x003
DOOR_GLOW = 0x201

TREE_TL = 0x1D4
TREE_TR = 0x1D5
TREE_BL = 0x1E4
TREE_BR = 0x1E5

raw = MAP.read_bytes()
expected = WIDTH * HEIGHT * 2
if len(raw) != expected:
    raise ValueError(f"Unexpected Mireglen map size: {len(raw)} (expected {expected})")

blocks = list(struct.unpack("<" + "H" * (WIDTH * HEIGHT), raw))

def index(x, y):
    return y * WIDTH + x

def get_block(x, y):
    return blocks[index(x, y)]

def set_block(x, y, metatile):
    i = index(x, y)
    blocks[i] = (blocks[i] & ~METATILE_MASK) | metatile

def copy_rect(x, y, w, h):
    return [[get_block(x + xx, y + yy) for xx in range(w)] for yy in range(h)]

def paste_rect(data, x, y):
    for yy, row in enumerate(data):
        for xx, value in enumerate(row):
            blocks[index(x + xx, y + yy)] = value

def place_tree(x, y):
    set_block(x, y, TREE_TL)
    set_block(x + 1, y, TREE_TR)
    set_block(x, y + 1, TREE_BL)
    set_block(x + 1, y + 1, TREE_BR)

# Preserve the authored building blocks before clearing Littleroot's old
# landscaping.  The two houses stay where they are; the lab moves three tiles
# east so the settlement has a sensible central spine.
left_house = copy_rect(2, 4, 5, 5)
right_house = copy_rect(13, 4, 5, 5)
lab = copy_rect(3, 12, 7, 5)

# Clear all outdoor decoration below the northern tree line.
for y in range(2, HEIGHT):
    for x in range(WIDTH):
        set_block(x, y, GRASS)

paste_rect(left_house, 2, 4)
paste_rect(right_house, 13, 4)
paste_rect(lab, 6, 12)

# House/lab signs and entrance glow tiles.
set_block(7, 8, SIGN)
set_block(12, 8, SIGN)
set_block(5, 9, DOOR_GLOW)
set_block(14, 9, DOOR_GLOW)
set_block(9, 17, SIGN)
set_block(10, 17, DOOR_GLOW)
set_block(13, 11, SIGN)

# Upper marsh arms.  These banks are real shoreline metatiles instead of
# isolated full-water tiles, so every transition joins cleanly.
for x in (0, 1):
    set_block(x, 2, WATER_TOP)
set_block(2, 2, WATER_TOP_RIGHT)
set_block(17, 2, WATER_TOP_LEFT)
for x in (18, 19):
    set_block(x, 2, WATER_TOP)

for y in range(3, 10):
    for x in (0, 1):
        set_block(x, y, REFLECTIVE_WATER)
    set_block(2, y, WATER_RIGHT_EDGE)
    set_block(17, y, WATER_LEFT_EDGE)
    for x in (18, 19):
        set_block(x, y, REFLECTIVE_WATER)

# The swamp widens inward below the houses.  Concave shoreline corners make
# this a deliberate natural inlet rather than a rectangular moat.
for x in (0, 1):
    set_block(x, 10, REFLECTIVE_WATER)
set_block(2, 10, WATER_INNER_RIGHT)
set_block(3, 10, WATER_TOP)
set_block(4, 10, WATER_TOP_RIGHT)

set_block(15, 10, WATER_TOP_LEFT)
set_block(16, 10, WATER_TOP)
set_block(17, 10, WATER_INNER_LEFT)
for x in (18, 19):
    set_block(x, 10, REFLECTIVE_WATER)

for y in range(11, 18):
    for x in range(0, 4):
        set_block(x, y, REFLECTIVE_WATER)
    set_block(4, y, WATER_RIGHT_EDGE)
    set_block(15, y, WATER_LEFT_EDGE)
    for x in range(16, 20):
        set_block(x, y, REFLECTIVE_WATER)

# Southern basin closes the U-shaped marsh around the central hummock.
for x in list(range(0, 4)) + list(range(16, 20)):
    set_block(x, 18, REFLECTIVE_WATER)
set_block(4, 18, WATER_TOP_RIGHT)
for x in range(5, 15):
    set_block(x, 18, WATER_TOP)
set_block(15, 18, WATER_TOP_LEFT)
for x in range(WIDTH):
    set_block(x, 19, REFLECTIVE_WATER)

# Vegetation is clustered along wet margins instead of scattered randomly.
place_tree(3, 2)
place_tree(15, 2)
place_tree(4, 10)
place_tree(14, 11)

for x, y in (
    (3, 4), (3, 5), (3, 9), (4, 9),
    (16, 4), (16, 5), (15, 9), (16, 9),
    (5, 11), (5, 12), (5, 13),
    (14, 13), (14, 14), (14, 15),
    (5, 16), (5, 17), (13, 17), (14, 17),
):
    set_block(x, y, TALL_GRASS)

for x, y in ((4, 8), (15, 8), (5, 10), (5, 15), (14, 16)):
    set_block(x, y, LONG_GRASS)

# Small maintained flower beds mark the inhabited centre of the swamp.
for x, y in ((8, 7), (8, 8), (8, 9), (11, 7), (11, 8), (11, 9)):
    set_block(x, y, FLOWER)

MAP.write_bytes(struct.pack("<" + "H" * len(blocks), *blocks))
print("Built cohesive Mireglen Swamp starting area.")
