#!/usr/bin/env python3
# Test01 finished starter art installer.
# The JSON files contain finalized 64x64 indexed pixel artwork derived from
# the approved concept art. This script only reconstructs the required PNG
# and palette formats; it does not procedurally draw the Pokémon.
from pathlib import Path
from PIL import Image
import base64
import json
import zlib

ROOT = Path(__file__).resolve().parents[1]
DATA_DIR = ROOT / "tools" / "test01_sprite_data"
SPECIES = ("torchic", "combusken", "blaziken", "mudkip", "marshtomp", "swampert")

def put_palette(image, palette):
    flat = []
    for r, g, b in palette:
        flat += [r, g, b]
    flat += [0] * (768 - len(flat))
    image.putpalette(flat)
    image.info["transparency"] = 0

def decode_sprite(encoded, palette):
    raw = zlib.decompress(base64.b85decode(encoded.encode("ascii")))
    if len(raw) != 64 * 64:
        raise ValueError(f"Unexpected decoded sprite size: {len(raw)}")
    image = Image.frombytes("P", (64, 64), raw)
    put_palette(image, palette)
    return image

def palette_text(palette):
    return "JASC-PAL\n0100\n16\n" + "\n".join(
        f"{r} {g} {b}" for r, g, b in palette
    ) + "\n"

def _components_for_index(image, palette_index):
    """Return 8-connected components for one palette index."""
    px = image.load()
    seen = set()
    components = []
    for y in range(64):
        for x in range(64):
            if px[x, y] != palette_index or (x, y) in seen:
                continue
            stack = [(x, y)]
            seen.add((x, y))
            component = []
            while stack:
                cx, cy = stack.pop()
                component.append((cx, cy))
                for dy in (-1, 0, 1):
                    for dx in (-1, 0, 1):
                        if dx == 0 and dy == 0:
                            continue
                        nx, ny = cx + dx, cy + dy
                        if (0 <= nx < 64 and 0 <= ny < 64
                                and (nx, ny) not in seen
                                and px[nx, ny] == palette_index):
                            seen.add((nx, ny))
                            stack.append((nx, ny))
            components.append(component)
    return components

def _all_nontransparent_components(image):
    px = image.load()
    seen = set()
    components = []
    for y in range(64):
        for x in range(64):
            if px[x, y] == 0 or (x, y) in seen:
                continue
            stack = [(x, y)]
            seen.add((x, y))
            component = []
            while stack:
                cx, cy = stack.pop()
                component.append((cx, cy))
                for dy in (-1, 0, 1):
                    for dx in (-1, 0, 1):
                        if dx == 0 and dy == 0:
                            continue
                        nx, ny = cx + dx, cy + dy
                        if (0 <= nx < 64 and 0 <= ny < 64
                                and (nx, ny) not in seen
                                and px[nx, ny] != 0):
                            seen.add((nx, ny))
                            stack.append((nx, ny))
            components.append(component)
    return sorted(components, key=len, reverse=True)

def clean_front_art(image, folder):
    """Remove the concept-art floor ellipse and conversion speckles.

    The previous test art had a painted oval/drop-shadow baked into the image.
    GBA battle sprites need a transparent floor; the battle engine supplies its
    own shadow separately.  Rules are intentionally species-specific so flame
    embers on the crab line are preserved.
    """
    rules = {
        "torchic":   (4, lambda n, box: n >= 200 and box[1] >= 45),
        "combusken": (4, lambda n, box: n >= 150 and box[1] >= 50),
        "blaziken":  (4, lambda n, box: box[1] >= 55 and n >= 3),
        "mudkip":    (3, lambda n, box: n >= 400 and box[1] >= 45),
        "marshtomp": (2, lambda n, box: box[1] >= 54 and n >= 8),
        "swampert":  (1, lambda n, box: box[1] >= 58 and n >= 3),
    }
    px = image.load()
    palette_index, predicate = rules[folder]
    for component in _components_for_index(image, palette_index):
        xs = [p[0] for p in component]
        ys = [p[1] for p in component]
        box = (min(xs), min(ys), max(xs), max(ys))
        if predicate(len(component), box):
            for x, y in component:
                px[x, y] = 0

    # Remove detached leftovers from the old floor ellipse.  Upper detached
    # pixels on the crab line are intentional embers and remain untouched.
    components = _all_nontransparent_components(image)
    if components:
        for component in components[1:]:
            min_y = min(y for _, y in component)
            remove = min_y >= 45
            if folder in ("mudkip", "marshtomp", "swampert"):
                remove = True
            if remove:
                for x, y in component:
                    px[x, y] = 0
    return image

for folder in SPECIES:
    data = json.loads((DATA_DIR / f"{folder}.json").read_text(encoding="utf-8"))
    palette = [tuple(c) for c in data["palette"]]
    shiny = [tuple(c) for c in data["shiny"]]
    front = clean_front_art(decode_sprite(data["front"], palette), folder)
    back = decode_sprite(data["back"], palette)

    out = ROOT / "graphics" / "pokemon" / folder
    out.mkdir(parents=True, exist_ok=True)

    anim = Image.new("P", (64, 128), 0)
    put_palette(anim, palette)
    anim.paste(front, (0, 0))
    anim.paste(front, (0, 64))
    for filename in ("anim_front.png", "anim_front_gba.png"):
        anim.save(out / filename, optimize=True)

    for filename in ("back.png", "back_gba.png"):
        back.save(out / filename, optimize=True)

    icon_frame = front.resize((32, 32), Image.Resampling.NEAREST)
    icon = Image.new("P", (32, 64), 0)
    put_palette(icon, palette)
    icon.paste(icon_frame, (0, 0))
    icon.paste(icon_frame, (0, 32))
    for filename in ("icon.png", "icon_gba.png"):
        icon.save(out / filename, optimize=True)

    # Temporary test overworld sheet derived from the authored battle art.
    # These can be replaced by dedicated walking sprites later.
    ow_frame = front.resize((30, 30), Image.Resampling.NEAREST)
    overworld = Image.new("P", (192, 32), 0)
    put_palette(overworld, palette)
    for i in range(6):
        overworld.paste(ow_frame, (i * 32 + 1, 1))
    overworld.save(out / "overworld.png", optimize=True)

    if data["female"]:
        anim.save(out / "anim_frontf.png", optimize=True)
        back.save(out / "backf.png", optimize=True)
        overworld.save(out / "overworldf.png", optimize=True)

    for filename, pal in (
        ("normal.pal", palette),
        ("normal_gba.pal", palette),
        ("shiny.pal", shiny),
        ("shiny_gba.pal", shiny),
        ("overworld_normal.pal", palette),
        ("overworld_shiny.pal", shiny),
    ):
        (out / filename).write_text(palette_text(pal), encoding="ascii")

print("Installed finalized Test01 starter artwork.")
