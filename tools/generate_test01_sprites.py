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

for folder in SPECIES:
    data = json.loads((DATA_DIR / f"{folder}.json").read_text(encoding="utf-8"))
    palette = [tuple(c) for c in data["palette"]]
    shiny = [tuple(c) for c in data["shiny"]]
    front = decode_sprite(data["front"], palette)
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
