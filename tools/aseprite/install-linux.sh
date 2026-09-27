#!/usr/bin/env bash
set -euo pipefail

ASEPRITE_TAG="${ASEPRITE_TAG:-v1.3.15.4}"
ROOT="${ASEPRITE_HOME:-${HOME}/.cache/test01-aseprite}"
SRC="$ROOT/src"
BUILD="$ROOT/build"
DEPS="$ROOT/deps"
SKIA="$DEPS/skia"

command -v git >/dev/null || { echo "git is required"; exit 1; }
command -v cmake >/dev/null || { echo "cmake is required"; exit 1; }
command -v ninja >/dev/null || { echo "ninja is required"; exit 1; }

mkdir -p "$ROOT" "$DEPS"

if [[ ! -d "$SRC/.git" ]]; then
  git clone --recursive --branch "$ASEPRITE_TAG" https://github.com/aseprite/aseprite.git "$SRC"
else
  git -C "$SRC" fetch --tags
  git -C "$SRC" checkout "$ASEPRITE_TAG"
  git -C "$SRC" submodule update --init --recursive
fi

if [[ ! -d "$SKIA" ]]; then
  echo "Skia is not installed at $SKIA."
  echo "Set SKIA_DIR to a compatible prebuilt Skia directory, or place it there."
  echo "See https://github.com/aseprite/aseprite/blob/main/INSTALL.md"
  exit 2
fi

cmake -S "$SRC" -B "$BUILD" -G Ninja   -DLAF_BACKEND=skia   -DSKIA_DIR="${SKIA_DIR:-$SKIA}"   -DSKIA_LIBRARY_DIR="${SKIA_LIBRARY_DIR:-${SKIA_DIR:-$SKIA}/out/Release-x64}"   -DSKIA_LIBRARY="${SKIA_LIBRARY:-skia}"
cmake --build "$BUILD" --target aseprite

"$BUILD/bin/aseprite" --version
echo "ASEPRITE_BIN=$BUILD/bin/aseprite"
