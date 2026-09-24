#!/usr/bin/env python3
"""Normalize pokeemerald's ARM/GAS battle-script assembly for Android/LLVM.

The battle scripts are data bytecode, but their source uses a few GNU ARM
assembler conveniences that Clang's AArch64 integrated assembler does not
accept:
  * @ comments
  * symbol:: global-label shorthand
  * .include files containing the same syntax
  * a trailing comma on one-argument macro invocations

The preprocessor also emits enum members as global absolute symbols. Those are
assembly-time constants, not linkable game data, so Android localizes them to
allow the two complete battle-script objects to coexist under lld.

This tool does not alter battle commands, operands, or script logic.
"""

from __future__ import annotations

import argparse
import re
from pathlib import Path

INCLUDE_RE = re.compile(r'^\s*\.include\s+"([^"]+)"\s*(?:@.*)?$')
GLOBAL_LABEL_RE = re.compile(
    r'^(?P<indent>\s*)(?P<label>[A-Za-z_.$][A-Za-z0-9_.$]*)::(?P<rest>.*)$'
)
TRAILING_MACRO_COMMA_RE = re.compile(
    r'^(?P<body>\s*[A-Za-z_.$][A-Za-z0-9_.$]*\s+[^,]+),\s*$'
)
SET_SYMBOL_RE = re.compile(
    r'^(?P<indent>\s*)\.set\s+(?P<symbol>[A-Za-z_.$][A-Za-z0-9_.$]*)\s*,'
)
ENUM_EQUIV_RE = re.compile(
    r'^(?P<indent>\s*)\.global\s+(?P<symbol>[A-Za-z_.$][A-Za-z0-9_.$]*)\s*;'
    r'\s*\.equiv\s+(?P=symbol)\s*,(?P<value>.*)$'
)


def strip_arm_comment(line: str) -> str:
    """Strip an ARM '@' comment while preserving @ inside quoted strings."""
    quote = None
    escaped = False
    for i, ch in enumerate(line):
        if escaped:
            escaped = False
            continue
        if ch == "\\":
            escaped = True
            continue
        if quote is not None:
            if ch == quote:
                quote = None
            continue
        if ch in ('"', "'"):
            quote = ch
            continue
        if ch == "@":
            return line[:i].rstrip()
    return line.rstrip("\n")


def expand_file(path: Path, repo_root: Path, stack: tuple[Path, ...]) -> list[str]:
    path = path.resolve()
    if path in stack:
        chain = " -> ".join(str(p) for p in (*stack, path))
        raise RuntimeError(f"recursive .include detected: {chain}")

    out: list[str] = []
    text = path.read_text(encoding="utf-8")
    for raw in text.splitlines():
        include_match = INCLUDE_RE.match(raw)
        if include_match:
            include_path = (repo_root / include_match.group(1)).resolve()
            if not include_path.is_file():
                raise FileNotFoundError(
                    f"{path}: assembler include not found: {include_match.group(1)}"
                )
            out.extend(expand_file(include_path, repo_root, (*stack, path)))
            continue

        line = strip_arm_comment(raw)

        # GNU as accepts a trailing comma on a one-argument macro invocation.
        # LLVM rejects the empty second argument.
        trailing_macro_comma = TRAILING_MACRO_COMMA_RE.match(line)
        if trailing_macro_comma:
            line = trailing_macro_comma.group("body")

        # preproc -ie emits enum members as:
        #   .global NAME; .equiv NAME, value
        # They are assembly-time constants rather than exported game symbols.
        enum_equiv_match = ENUM_EQUIV_RE.match(line)
        if enum_equiv_match:
            indent = enum_equiv_match.group("indent")
            symbol = enum_equiv_match.group("symbol")
            value = enum_equiv_match.group("value")
            line = f"{indent}.local {symbol}; .equiv {symbol},{value}"

        # Keep support for ordinary .set constants as well.
        set_match = SET_SYMBOL_RE.match(line)
        if set_match and not set_match.group("symbol").startswith(".L"):
            out.append(
                f'{set_match.group("indent")}.local {set_match.group("symbol")}'
            )

        label_match = GLOBAL_LABEL_RE.match(line)
        if label_match:
            indent = label_match.group("indent")
            label = label_match.group("label")
            rest = label_match.group("rest")
            out.append(f"{indent}.global {label}")
            out.append(f"{indent}{label}:{rest}")
        else:
            out.append(line)

    return out


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("input", type=Path)
    parser.add_argument("output", type=Path)
    parser.add_argument("--repo-root", type=Path, required=True)
    args = parser.parse_args()

    repo_root = args.repo_root.resolve()
    source = args.input.resolve()
    output = args.output.resolve()

    lines = expand_file(source, repo_root, ())
    output.parent.mkdir(parents=True, exist_ok=True)

    # Both battle-script and battle-animation bytecode contain absolute
    # pointers. The Android runtime is 64-bit, so expose an assembler symbol
    # consumed by bscript_ptr/animscript_ptr before LLVM expands the macros.
    prefix = [
        ".set PLATFORM_ANDROID_PTR64, 1",
        "",
    ]
    output.write_text("\n".join(prefix + lines) + "\n", encoding="utf-8")


if __name__ == "__main__":
    main()
