# Project Aseprite toolchain

This directory is the durable bootstrap point for Aseprite used by the Test01 / Pokemon Emerald GBA art pipeline.

## For ChatGPT project containers

1. Run `bash tools/aseprite/verify.sh`.
2. If Aseprite is missing, run `bash tools/aseprite/install-linux.sh`.
3. The default executable is `~/.cache/test01-aseprite/build/bin/aseprite`.
4. Override it with `ASEPRITE_BIN` or the installation root with `ASEPRITE_HOME`.

The build is intentionally kept out of git. Containers are ephemeral; these scripts are the persistent source of truth.

Aseprite requires its compatible Skia dependency. The installer expects Skia at `~/.cache/test01-aseprite/deps/skia` or via `SKIA_DIR`. Do not commit Aseprite/Skia binaries or source into this repository.

## GBA / Pokemon Emerald output contract

Pipeline-generated Pokemon sprites should be validated before import:
- 64x64 canvas where required by the target Emerald asset.
- Indexed color.
- 4bpp / maximum 16 palette entries including transparency.
- Binary transparency.
- No antialiasing or semi-transparent pixels.
- Preserve exact pixel boundaries and frame dimensions expected by the decomp asset being replaced.

## Future-chat instruction

When working on sprite generation in this Project, do not assume a previous chat's container survived. Run `tools/aseprite/verify.sh` first. If it fails, restore the toolchain using this directory before generating or validating sprite assets.
