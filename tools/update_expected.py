#!/usr/bin/env python3
"""Snapshot verified objects and update objdiff metadata."""

import shutil
import sys
from pathlib import Path

from validate_commit import verify_image

ROOT = Path(__file__).resolve().parent.parent
OUT = ROOT / "build"


def update_expected():
    """Snapshot build/src objects as objdiff targets (run from a verified
    byte-matching build) and regenerate objdiff.json's unit list."""
    import json

    # Removed or merged source files must not remain as stale objdiff units.
    sources = sorted(src for src in (ROOT / "src").rglob("*")
                     if src.is_file() and src.suffix in (".c", ".s"))
    src_objs = [OUT / src.relative_to(ROOT).with_suffix(".o") for src in sources]
    if not src_objs:
        sys.exit("no build/src objects; run a full build first")
    if any(not obj.exists() for obj in src_objs):
        sys.exit("missing build/src objects; run a full build first")
    active = {obj.relative_to(OUT) for obj in src_objs}
    for stale in (ROOT / "expected" / "src").rglob("*.o"):
        if stale.relative_to(ROOT / "expected") not in active:
            stale.unlink()
    units = []
    for source, obj in zip(sources, src_objs):
        rel = obj.relative_to(OUT)
        dst = ROOT / "expected" / rel
        dst.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(obj, dst)
        unit = rel.relative_to("src").with_suffix("").as_posix()
        units.append({
            "name": unit,
            "target_path": f"expected/{rel.as_posix()}",
            "base_path": f"build/compare/{rel.as_posix()}",
            "metadata": {"source_path": source.relative_to(ROOT).as_posix()},
        })
    cfg_path = ROOT / "objdiff.json"
    cfg = json.loads(cfg_path.read_text())
    cfg["custom_make"] = "make"
    cfg["custom_args"] = ["COMPARISON=1", "BUILD_DIR=build/compare"]
    for pattern in ("Makefile", "*.mk"):
        if pattern not in cfg.setdefault("watch_patterns", []):
            cfg["watch_patterns"].append(pattern)
    cfg["units"] = units
    cfg_path.write_text(json.dumps(cfg, indent=2) + "\n")
    print(f"expected/: {len(units)} units snapshotted; objdiff.json updated")



if __name__ == "__main__":
    verify_image(ROOT)
    update_expected()
