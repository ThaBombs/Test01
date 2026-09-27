#!/usr/bin/env bash
ROOT="${ASEPRITE_HOME:-${HOME}/.cache/test01-aseprite}"
export ASEPRITE_BIN="${ASEPRITE_BIN:-$ROOT/build/bin/aseprite}"
if [[ ! -x "$ASEPRITE_BIN" ]]; then
  echo "Aseprite is unavailable: $ASEPRITE_BIN" >&2
  echo "Restore it with tools/aseprite/install-linux.sh" >&2
  return 1 2>/dev/null || exit 1
fi
echo "Using Aseprite: $ASEPRITE_BIN"
