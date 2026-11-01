#!/usr/bin/env python3
"""Remove generated function disassemblies before regenerating a split."""

import shutil
from pathlib import Path


def clear_generated_asm(root: Path) -> None:
    for name in ("nonmatchings", "matchings"):
        path = root / "asm" / name
        if path.is_symlink():
            raise ValueError(f"refusing to remove symlink: {path}")
        if path.exists():
            shutil.rmtree(path)


if __name__ == "__main__":
    clear_generated_asm(Path(__file__).resolve().parent.parent)
