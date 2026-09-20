#!/usr/bin/env python3
# Test01 swamp layout patch.
# Keeps Littleroot's event/warp coordinates intact but turns the first outdoor
# area into a wet, overgrown swamp for map-editing tests.
from pathlib import Path
import struct

ROOT = Path(__file__).resolve().parents[1]
MAP = ROOT / "data" / "layouts" / "LittlerootTown" / "map.bin"
WIDTH = 20
HEIGHT = 20

METATILE_MASK = 0x03FF
GRASS = 0x001
TALL_GRASS = 0x00D
LONG_GRASS = 0x015
CALM_WATER = 0x170
ROCKY_SOIL = 0x206

raw = bytearray(MAP.read_bytes())
expected = WIDTH * HEIGHT * 2
if len(raw) != expected:
    raise ValueError(f"Unexpected Littleroot map size: {len(raw)} (expected {expected})")

blocks = list(struct.unpack("<" + "H" * (WIDTH * HEIGHT), raw))

# Irregular pools placed away from houses, lab doors, truck exits and story triggers.
water = set()
for y in range(2, 8):
    for x in range(0, 5):
        if (x + y) % 4 != 0:
            water.add((x, y))
for y in range(11, 20):
    for x in range(16, 20):
        if (x * 2 + y) % 5 != 0:
            water.add((x, y))
for y in range(0, 3):
    for x in range(13, 20):
        if (x + y) % 3:
            water.add((x, y))

tall = {
    (4,2),(5,2),(4,3),(5,4),(3,8),(4,8),(5,8),
    (15,10),(16,10),(15,11),(14,12),(15,13),(15,14),
    (12,3),(13,3),(14,3),(18,3),(19,3),
    (1,8),(2,8),(3,9),(4,9),(15,17),(15,18)
}
long_grass = {
    (5,3),(4,4),(5,5),(4,6),(5,7),
    (14,11),(14,13),(14,15),(15,16),(14,18),
    (12,2),(12,4),(13,4),(18,4)
}
mud = {
    (5,6),(6,6),(4,10),(5,10),(15,9),(16,9),
    (13,17),(14,17),(12,18),(13,18)
}

def replace_if_open(x, y, tile):
    i = y * WIDTH + x
    old = blocks[i]
    if old & METATILE_MASK == GRASS:
        blocks[i] = (old & ~METATILE_MASK) | tile

for x,y in water:
    replace_if_open(x,y,CALM_WATER)
for x,y in tall:
    replace_if_open(x,y,TALL_GRASS)
for x,y in long_grass:
    replace_if_open(x,y,LONG_GRASS)
for x,y in mud:
    replace_if_open(x,y,ROCKY_SOIL)

MAP.write_bytes(struct.pack("<" + "H" * len(blocks), *blocks))
print("Applied Test01 swamp terrain to LittlerootTown.")
