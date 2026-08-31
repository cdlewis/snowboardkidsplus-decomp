#!/usr/bin/env python3
"""Identify PSY-Q SDK library code in the boot exe via psx_psyq_signatures.

For every SDK version in tools/psx_psyq_signatures/, mask-match each library
module's signature against the exe and score the version by matched bytes.
Report the winning version's module map (address ranges + function labels).

Usage:
    python tools/find_psyq.py            # score all versions
    python tools/find_psyq.py 430        # detail one version (module map)
    python tools/find_psyq.py 430 --syms # also print symbol_addrs lines
"""

import json
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
SIGS = ROOT / "tools" / "psx_psyq_signatures"
EXE = ROOT / "disc" / "SLPS_018.23"

VRAM_BASE = 0x80010000
FILE_BASE = 0x800

data = EXE.read_bytes()


def parse_sig(sig_str):
    toks = sig_str.split()
    pat = bytearray()
    mask = bytearray()
    for t in toks:
        if t == "??":
            pat.append(0)
            mask.append(0)
        else:
            pat.append(int(t, 16))
            mask.append(0xFF)
    return bytes(pat), bytes(mask)


def find_masked(pat, mask, start=FILE_BASE):
    """Yield file offsets (word-aligned) where pat/mask matches."""
    n = len(pat)
    # anchor: longest fully-fixed run to use bytes.find
    best_off, best_len, i = 0, 0, 0
    while i < n:
        if mask[i] == 0xFF:
            j = i
            while j < n and mask[j] == 0xFF:
                j += 1
            if j - i > best_len:
                best_off, best_len = i, j - i
            i = j
        else:
            i += 1
    anchor = pat[best_off:best_off + best_len]
    pos = start
    while True:
        hit = data.find(anchor, pos)
        if hit < 0:
            return
        cand = hit - best_off
        pos = hit + 1
        if cand < 0 or cand + n > len(data) or cand % 4 != 0:
            continue
        if all(data[cand + k] & mask[k] == pat[k] for k in range(n)):
            yield cand


def load_version(ver):
    mods = []
    for f in sorted((SIGS / ver).glob("*.json")):
        try:
            entries = json.load(open(f))
        except Exception:
            continue
        if not isinstance(entries, list):
            continue
        for e in entries:
            if not isinstance(e, dict) or "sig" not in e:
                continue
            pat, mask = parse_sig(e["sig"])
            if len(pat) < 16:
                continue
            mods.append((f.stem, e.get("name", "?"), pat, mask,
                         e.get("labels", [])))
    return mods


def scan_version(ver):
    hits = []
    for lib, obj, pat, mask, labels in load_version(ver):
        for off in find_masked(pat, mask):
            hits.append((off, len(pat), lib, obj, labels))
            break  # a module links at most once
    return hits


def main():
    detail_ver = next((a for a in sys.argv[1:] if not a.startswith("-")), None)
    versions = ([detail_ver] if detail_ver else
                sorted(d.name for d in SIGS.iterdir()
                       if d.is_dir() and d.name.isdigit()))

    results = {}
    for ver in versions:
        hits = scan_version(ver)
        matched = sum(h[1] for h in hits)
        results[ver] = hits
        print(f"psyq {ver}: {len(hits):4d} modules, {matched:7d} bytes matched")

    if not detail_ver:
        best = max(results, key=lambda v: sum(h[1] for h in results[v]))
        print(f"\nbest: {best} (rerun with `{sys.argv[0]} {best}` for the map)")
        return

    hits = sorted(results[detail_ver])
    print(f"\n=== module map (psyq {detail_ver}) ===")
    for off, size, lib, obj, labels in hits:
        vram = VRAM_BASE + off - FILE_BASE
        print(f"0x{off:06X}  0x{vram:08X}  {size:6d}  {lib:14s} {obj}")
    if hits:
        lo, hi = hits[0][0], max(h[0] + h[1] for h in hits)
        print(f"\nrange: file 0x{lo:X}..0x{hi:X} "
              f"(vram 0x{VRAM_BASE + lo - FILE_BASE:X}..0x{VRAM_BASE + hi - FILE_BASE:X})")

    if "--syms" in sys.argv:
        # confidence filter: drop offsets claimed by modules with differing
        # label sets (ambiguous trivial stubs), and lone tiny matches far from
        # any other module (false positives in game code).
        by_off = {}
        for h in hits:
            by_off.setdefault(h[0], []).append(h)
        offsets = sorted(by_off)
        print("\n=== symbol_addrs lines ===")
        seen = {}
        for off in offsets:
            claims = by_off[off]
            labelsets = {tuple(sorted(l["name"] for l in h[4])) for h in claims}
            if len(labelsets) > 1:
                continue  # ambiguous
            size = claims[0][1]
            near = any(abs(o - off) <= 0x2000 for o in offsets if o != off)
            if size < 48 and not near:
                continue  # isolated stub
            for lab in claims[0][4]:
                vram = VRAM_BASE + off - FILE_BASE + lab["offset"]
                if seen.setdefault(vram, lab["name"]) != lab["name"]:
                    print(f"// CONFLICT at 0x{vram:08X}: "
                          f"{seen[vram]} vs {lab['name']}")
                    continue
                print(f"{lab['name']} = 0x{vram:08X}; // type:func")


if __name__ == "__main__":
    main()
