#!/usr/bin/env bash
set -euo pipefail
ROOT="${ASEPRITE_HOME:-${HOME}/.cache/test01-aseprite}"
BIN="${ASEPRITE_BIN:-$ROOT/build/bin/aseprite}"
if [[ ! -x "$BIN" ]]; then
  echo "Aseprite not found at $BIN"
  echo "Run tools/aseprite/install-linux.sh first."
  exit 1
fi
"$BIN" --version
