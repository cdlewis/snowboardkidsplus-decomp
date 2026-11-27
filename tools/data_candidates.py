#!/usr/bin/env python3
"""Emit remaining assembly-owned data/rodata subsegments in ROM order."""

import argparse
import json
from pathlib import Path

import yaml


ROOT = Path(__file__).resolve().parent.parent


def remaining_segments(layout):
    segments = layout["segments"]
    for index, segment in enumerate(segments):
        if not isinstance(segment, dict):
            continue
        following = segments[index + 1] if index + 1 < len(segments) else None
        segment_end = (following.get("start") if isinstance(following, dict)
                       else following[0] if following else None)
        subs = segment.get("subsegments", [])
        for sub_index, sub in enumerate(subs):
            if isinstance(sub, dict):
                start, kind, name = sub.get("start"), sub.get("type"), sub.get("name")
            else:
                start = sub[0]
                kind = sub[1] if len(sub) > 1 else None
                name = sub[2] if len(sub) > 2 else None
            if kind not in {"data", "rodata"}:
                continue
            following_sub = subs[sub_index + 1] if sub_index + 1 < len(subs) else None
            end = (following_sub.get("start") if isinstance(following_sub, dict)
                   else following_sub[0] if following_sub else segment_end)
            if not isinstance(start, int) or not isinstance(end, int) or end <= start:
                raise ValueError(f"Cannot determine ROM bounds for {kind} at {start!r}")
            yield {"candidate": f"{kind}@0x{start:X}", "start": start,
                   "end": end, "size": end - start, "name": name,
                   "segment": segment.get("name")}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--details", action="store_true", help="include ROM bounds and names")
    parser.add_argument("--limit", type=int, help="limit candidates; default: all")
    args = parser.parse_args()
    if args.limit is not None and args.limit < 1:
        parser.error("limit must be positive")
    layout = yaml.safe_load((ROOT / "snowboardkidsplus.yaml").read_text())
    rows = sorted(remaining_segments(layout), key=lambda row: row["start"])[:args.limit]
    print(json.dumps(rows if args.details else [row["candidate"] for row in rows], indent=2))


if __name__ == "__main__":
    main()
