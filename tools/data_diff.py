#!/usr/bin/env python3
"""Compare original PS-X EXE data and symbol placement with a fresh build.

Adapted from both sibling data-differ workflows. PSX load addresses come from
the executable header, and values are little endian. BSS is never byte-scored.
Exit codes: 0 equal, 1 mismatch, 2 missing/unsupported/stale input.
"""

import argparse
import re
import subprocess
from pathlib import Path

from decomp import ROOT, Elf32, PsxImage, configured_symbols, first_difference, require_fresh


def compare_symbol(name, expected, actual, symbols, configured, size=None):
    if name not in symbols:
        raise ValueError(f"symbol is absent from the linked ELF: {name}")
    linked = symbols[name]
    spec = configured.get(name)
    encoded = re.match(r'^D_([0-9A-Fa-f]{8})(?:_|$)', name)
    address = spec.address if spec else int(encoded[1], 16) if encoded else None
    if address is None:
        raise ValueError(f"original address unknown for {name}; add symbol metadata first")
    size = size if size is not None else spec.size if spec and spec.size else linked.size
    if not size:
        raise ValueError(f"size unknown for {name}; provide --size or configs/symbol_addrs.txt metadata")
    target = expected.read(address, size)
    built = actual.read(linked.address, size)
    size_matches = not linked.size or linked.size == size
    return address == linked.address and size_matches and target == built, address, linked.address, target, built


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("symbol", nargs="?")
    parser.add_argument("--size", type=lambda s: int(s, 0))
    parser.add_argument("--find-first-mismatch", action="store_true",
                        help="compare the entire executable, including its header and trailing bytes")
    parser.add_argument("--rebuild", action="store_true")
    parser.add_argument("--target", type=Path, default=ROOT / "disc/SLPS_018.23")
    parser.add_argument("--built", type=Path, default=ROOT / "build/slps_018.23.exe")
    parser.add_argument("--elf", type=Path, default=ROOT / "build/slps_018.23.elf")
    args = parser.parse_args()
    if bool(args.symbol) == args.find_first_mismatch:
        parser.error("choose a symbol or --find-first-mismatch")
    if args.rebuild:
        subprocess.run(["make"], cwd=ROOT, check=True)
    require_fresh()
    expected, actual = PsxImage(args.target.read_bytes()), PsxImage(args.built.read_bytes())
    configured = configured_symbols()
    if args.find_first_mismatch:
        offset = first_difference(expected.data, actual.data)
        if offset is None:
            print(f"{args.built}: all {len(actual.data)} bytes match")
            return 0
        print(f"first image mismatch at file offset 0x{offset:X}; "
              f"original size 0x{len(expected.data):X}, built size 0x{len(actual.data):X}")
        if 0x800 <= offset < 0x800 + expected.size:
            address = expected.address + offset - 0x800
            print(f"original VRAM 0x{address:08X}")
            for symbol in configured.values():
                if symbol.size and symbol.address <= address < symbol.address + symbol.size:
                    print(f"inside {symbol.name} + 0x{address - symbol.address:X}")
        for label, data in [("original", expected.data), ("built", actual.data)]:
            print(f"{label:8}: {data[offset:offset + 32].hex(' ')}")
        return 1
    symbols = Elf32(args.elf).symbols
    equal, address, linked, target, built = compare_symbol(
        args.symbol, expected, actual, symbols, configured, args.size)
    print(f"{args.symbol}: {'MATCH' if equal else 'MISMATCH'}; {len(target)} bytes; "
          f"original 0x{address:08X}, linked 0x{linked:08X}")
    if symbols[args.symbol].size and symbols[args.symbol].size != len(target):
        print(f"linked symbol size {symbols[args.symbol].size} differs from expected {len(target)}")
    offset = first_difference(target, built)
    if offset is not None:
        print(f"first byte difference +0x{offset:X}")
        print(f"original: {target[offset:offset + 32].hex(' ')}")
        print(f"built:    {built[offset:offset + 32].hex(' ')}")
    return 0 if equal else 1


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (ValueError, OSError, KeyError, subprocess.CalledProcessError) as error:
        print(error)
        raise SystemExit(2)
