#!/usr/bin/env python3
"""Normalize pokeemerald's ARM/GAS battle-script assembly for Android/LLVM.

The battle scripts are data bytecode, but their source uses a few GNU ARM
assembler conveniences that Clang's AArch64 integrated assembler does not
accept:
  * @ comments
  * symbol:: global-label shorthand
  * .include files containing the same syntax

This tool expands assembler .include directives recursively and normalizes
those syntax differences. C-preprocessor directives are deliberately kept so
the generated .S file can still be compiled with -x assembler-with-cpp.

It does not alter battle commands, operands, or script logic.
"""

from __future__ import annotations

import argparse
import re
from pathlib import Path

INCLUDE_RE = re.compile(r'^\s*\.include\s+"([^"]+)"\s*(?:@.*)?$')
GLOBAL_LABEL_RE = re.compile(
    r'^(?P<indent>\s*)(?P<label>[A-Za-z_.$][A-Za-z0-9_.$]*)::(?P<rest>.*)$'
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
    output.write_text("\n".join(lines) + "\n", encoding="utf-8")


if __name__ == "__main__":
    main()
