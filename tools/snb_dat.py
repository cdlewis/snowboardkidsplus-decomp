#!/usr/bin/env python3
"""Lossless SNB.DAT catalogue, Splat configuration, C references and verification."""

import argparse
from collections import Counter
import hashlib
import json
from pathlib import Path
import re
import struct

import yaml

ROOT = Path(__file__).resolve().parent.parent
MANIFEST = ROOT / "config/dat_resources.yaml"
INPUT = ROOT / "disc/SNB/SNB.DAT"
OUTPUT = ROOT / "build/dat"
SECTOR = 0x800
# Independent of the editable resource manifest. The commit gate protects this value.
DAT_SHA1 = "226b3cd476010a340455b9720b2e37dcd94b7933"
DAT_SIZE = 0x11BC000
DAT_COUNT = 395


def require(condition, message):
    if not condition:
        raise ValueError(message)


def write_text(path, text):
    path.parent.mkdir(parents=True, exist_ok=True)
    if not path.exists() or path.read_text() != text:
        path.write_text(text)


def read_archive(path=INPUT):
    data = path.read_bytes()
    require(len(data) == DAT_SIZE, f"{path}: unexpected DAT size")
    require(hashlib.sha1(data).hexdigest() == DAT_SHA1, f"{path}: DAT SHA1 mismatch")
    count = struct.unpack_from("<I", data)[0]
    require(count == DAT_COUNT, "unexpected DAT directory count")
    offsets = list(struct.unpack_from(f"<{count}I", data, 4))
    ends = offsets[1:] + [len(data)]
    require(offsets[0] >= 4 + count * 4, "DAT directory overlaps payload")
    require(all(0 <= a < b <= len(data) and a % SECTOR == b % SECTOR == 0
                for a, b in zip(offsets, ends)), "invalid DAT entry bounds/alignment")
    return data, offsets, ends


def tim_length(data):
    """Return a structurally validated TIM length, excluding sector padding."""
    if len(data) < 8:
        return None
    magic, flags = struct.unpack_from("<II", data)
    if magic != 0x10 or flags & ~0xF or (flags & 7) > 3:
        return None
    pos = 8
    for _ in range(2 if flags & 8 else 1):
        if pos + 12 > len(data):
            return None
        size, _, _, width, height = struct.unpack_from("<I4H", data, pos)
        if size != 12 + width * height * 2 or pos + size > len(data):
            return None
        pos += size
    return pos


def describe(data, known_archive=False):
    length = tim_length(data)
    if length is not None:
        return {"format": "tim", "payload_size": length,
                "evidence": "TIM header and CLUT/pixel block lengths validated"}
    if data[:4] == b"pBAV" and len(data) >= 0x20:
        return {"format": "vab_header", "evidence": "pBAV signature; body pairing requires loader evidence"}
    if data[:4] == b"pQES":
        return {"format": "sequence", "evidence": "pQES signature"}
    count = struct.unpack_from("<I", data)[0] if len(data) >= 4 else 0
    if 0 < count <= (len(data) - 4) // 4:
        raw = list(struct.unpack_from(f"<{count}I", data, 4))
        offsets = [value & ~1 for value in raw]
        if (offsets[0] >= (count + 1) * 4 and
                all(a < b for a, b in zip(offsets, offsets[1:] + [len(data)]))):
            children = []
            for index, (start, end) in enumerate(zip(offsets, offsets[1:] + [len(data)])):
                child_length = tim_length(data[start:end])
                child = {"index": index, "offset_word": raw[index], "offset": start,
                         "allocated_size": end - start, "format": "tim" if child_length else "unknown"}
                if child_length:
                    child["payload_size"] = child_length
                children.append(child)
            return {"format": "offset_archive" if known_archive else "offset_archive_candidate",
                    "evidence": "consumer-confirmed offsets" if known_archive else "bounded ascending count/offset structure only",
                    "children": children}
    return {"format": "unknown", "evidence": "preserved without a format assumption"}


def load_manifest():
    manifest = yaml.safe_load(MANIFEST.read_text())
    entries = manifest["entries"]
    require(sorted(entries) == list(range(DAT_COUNT)), "manifest must describe all 395 entry indices")
    names = [e["name"] for e in entries.values()]
    require(len(set(names)) == DAT_COUNT, "duplicate resource names")
    require(all(re.fullmatch(r"[a-z][a-z0-9_]*", name) for name in names), "invalid resource name")
    return manifest


def runtime_symbols(manifest):
    text = (ROOT / "symbol_addrs.txt").read_text()
    result = []
    macros = [region["macro"] for region in manifest["runtime_regions"]]
    require(len(set(macros)) == len(macros), "duplicate runtime macro")
    require(all(re.fullmatch(r"[A-Z][A-Z0-9_]*", macro) for macro in macros), "invalid runtime macro")
    for region in manifest["runtime_regions"]:
        symbol = region["symbol"]
        matches = re.findall(rf"^{re.escape(symbol)}\s*=\s*(0x[\da-fA-F]+);([^\n]*)", text, re.M)
        require(len(matches) == 1, f"expected one canonical symbol for {symbol}")
        address = int(matches[0][0], 16)
        require(re.search(r"\babsolute:\s*true\b", matches[0][1]) is not None and
                re.search(r"\bextract:\s*false\b", matches[0][1]) is not None,
                f"{symbol}: runtime views must be absolute and non-extracted")
        size = re.search(r"size:(0x[\da-fA-F]+|\d+)", matches[0][1])
        require(size is not None, f"missing runtime extent for {symbol}")
        size = int(size[1], 0)
        require(0x80000000 <= address < address + size <= 0x80200000, f"invalid RAM extent: {symbol}")
        result.append(dict(region, address=address, size=size))
    return result


def catalogue():
    data, offsets, ends = read_archive()
    manifest = load_manifest()
    result = []
    for index, (start, end) in enumerate(zip(offsets, ends)):
        metadata = manifest["entries"][index]
        resource = {"index": index, **metadata, "offset": start, "end": end,
                    "sector": start // SECTOR, "sector_count": (end - start) // SECTOR,
                    "allocated_size": end - start,
                    "sha1": hashlib.sha1(data[start:end]).hexdigest(),
                    **describe(data[start:end], metadata.get("archive_confirmed", False))}
        resource["file"] = f"entry_{index:03d}_{metadata['name']}.bin"
        result.append(resource)
    return data, result, manifest


def constants(entries, regions):
    values = {}
    for entry in entries:
        prefix = f"DAT_{entry['name'].upper()}"
        values[prefix + "_SECTOR"] = entry["sector"]
        values[prefix + "_SECTOR_COUNT"] = entry["sector_count"]
    for region in regions:
        values[region["macro"] + "_ADDRESS"] = region["address"]
    return values


def literal(expression, values):
    expression = expression.strip()
    if expression in values:
        return values[expression]
    if re.fullmatch(r"-?(?:0[xX][\da-fA-F]+|\d+)", expression):
        return int(expression, 0)
    return None


def intersections(sector, count, entries):
    require(sector >= 0 and count > 0, f"invalid read: {sector}, {count}")
    start, end = sector * SECTOR, (sector + count) * SECTOR
    require(end <= DAT_SIZE, f"read exceeds DAT: sector {sector:#x}, count {count:#x}")
    hits = [{"entry": e["index"], "entry_offset": max(start, e["offset"]) - e["offset"],
             "size": min(end, e["end"]) - max(start, e["offset"])}
            for e in entries if e["offset"] < end and start < e["end"]]
    require(sum(h["size"] for h in hits) == end - start, "read intersects DAT directory")
    whole = len(hits) == 1 and hits[0]["entry_offset"] == 0 and hits[0]["size"] == entries[hits[0]["entry"]]["allocated_size"]
    return {"sector": sector, "sector_count": count, "entries": hits,
            "coverage": "whole_entry" if whole else "multiple_entries" if len(hits) > 1 else "partial_entry"}


def call_arguments(text, start):
    """Split C arguments without splitting nested calls, subscripts or casts."""
    depth, begin, args = 0, start, []
    for pos in range(start, len(text)):
        char = text[pos]
        if char in "([{":
            depth += 1
        elif char in ")]}":
            if char == ")" and depth == 0:
                return args + [text[begin:pos].strip()]
            depth -= 1
        elif char == "," and depth == 0:
            args.append(text[begin:pos].strip())
            begin = pos + 1
    raise ValueError("unterminated C call")


def c_reads(entries, manifest, values):
    from decomp import uncomment

    reads = []
    for path in sorted((ROOT / "src").rglob("*.c")):
        text = uncomment(path.read_text())
        for match in re.finditer(r"\bqueueCdSectorCommand\s*\(", text):
            args = call_arguments(text, match.end())
            if len(args) != 5:  # definition, which also has five parameters
                raise ValueError(f"unexpected queue call in {path}")
            if args[0].startswith("s16 "):
                continue
            record = {"source": str(path.relative_to(ROOT)), "line": text.count("\n", 0, match.start()) + 1,
                      "kind": "c_call", "arguments": args, "command": literal(args[0], values),
                      "destination": args[3]}
            sector, count = (literal(a, values) for a in args[1:3])
            if sector is not None and count is not None:
                record.update(intersections(sector, count, entries))
            else:
                record["coverage"] = "table_driven" if any(t["name"] in args[1] for t in manifest["read_tables"]) else "unresolved"
            reads.append(record)
    for table in manifest["read_tables"]:
        path = ROOT / table["source"]
        text = uncomment(path.read_text())
        match = re.search(rf"\b{table['name']}\s*\[.*?=\s*\{{(.*?)\n\}};", text, re.S)
        require(match is not None, f"missing read table {table['name']}")
        rows = re.findall(r"\{([^{}]+)\}", match[1])
        require(len(rows) == table["rows"], f"changed row count for {table['name']}")
        for index, row in enumerate(rows):
            cells = row.split(",")
            sector, count = (literal(cells[field], values) for field in table["fields"])
            require(sector is not None and count is not None, f"unresolved row in {table['name']}")
            record = {"source": table["source"], "kind": "table_row", "table": table["name"],
                      "row": index, "uses": table["uses"]}
            if sector == count == 0:
                record.update(coverage="absent", sector=0, sector_count=0)
            else:
                record.update(intersections(sector, count, entries))
            reads.append(record)
    return reads


def asm_reads(entries):
    """Conservative constant propagation within basic blocks, including delay slots.

    Unknown loads/joins remain unresolved; the report retains the instructions.
    This inventories live fallbacks only, avoiding duplicate matched C reads.
    """
    reads = []
    from decomp import INCLUDE, uncomment

    live = set()
    for path in (ROOT / "src").rglob("*.c"):
        for match in INCLUDE.finditer(uncomment(path.read_text())):
            live.add(ROOT / match[1] / (match[2] + ".s"))
    for path in sorted(live):
        lines = path.read_text().splitlines()
        regs = {0: 0}
        pending = None
        reset_after_delay = False
        for index, line in enumerate(lines):
            if re.match(r"(?:glabel|\.L\w+:)", line.strip()):
                regs = {0: 0}
            match = re.search(r"/\*\s+[\dA-F]+\s+([\dA-F]+)\s+([\dA-F]{8})\s+\*/", line)
            if not match:
                continue
            word = int.from_bytes(bytes.fromhex(match[2]), "little")
            op, rs, rt, rd, fn = word >> 26, (word >> 21) & 31, (word >> 16) & 31, (word >> 11) & 31, word & 63
            imm = word & 0xFFFF
            signed = imm if imm < 0x8000 else imm - 0x10000
            old_pending, pending = pending, None
            old_reset, reset_after_delay = reset_after_delay, False
            if op == 15:
                regs[rt] = imm << 16
            elif op in (9, 13):
                value = regs.get(rs)
                regs[rt] = None if value is None else ((value + signed) if op == 9 else value | imm) & 0xFFFFFFFF
            elif op == 0 and fn in (0x21, 0x25):
                a, b = regs.get(rs), regs.get(rt)
                regs[rd] = None if a is None or b is None else ((a + b) if fn == 0x21 else a | b) & 0xFFFFFFFF
            elif op == 0 and fn == 0:
                a = regs.get(rt)
                regs[rd] = None if a is None else (a << ((word >> 6) & 31)) & 0xFFFFFFFF
            elif op == 0:
                if fn not in (8, 9, 0x18, 0x19, 0x1A, 0x1B):
                    regs[rd] = None
            elif op not in (2, 3, 4, 5, 6, 7, 1, 0x28, 0x29, 0x2B, 0x2A, 0x2E):
                regs[rt] = None
            regs[0] = 0
            if old_pending is not None:
                if old_pending["queue"]:
                    record = {"source": str(path.relative_to(ROOT)), "kind": "asm_call",
                              "address": old_pending["address"], "command": regs.get(4),
                              "destination": regs.get(7), "context": lines[max(0, index - 10):index + 1]}
                    sector, count = regs.get(5), regs.get(6)
                    if sector is not None and count is not None:
                        record.update(intersections(sector, count, entries))
                    else:
                        record["coverage"] = "unresolved"
                    reads.append(record)
                # A callee may overwrite all caller-saved registers.
                regs = {r: v for r, v in regs.items() if r == 0 or 16 <= r <= 23 or r in (28, 29, 30)}
            if old_reset:
                regs = {0: 0}
            if op == 3 or (op == 0 and fn == 9):
                pending = {"queue": "queueCdSectorCommand" in line, "address": int(match[1], 16)}
            elif op in (1, 2, 4, 5, 6, 7) or (op == 0 and fn == 8):
                reset_after_delay = True
    return reads


def inventory(entries, manifest, regions):
    reads = c_reads(entries, manifest, constants(entries, regions)) + asm_reads(entries)
    executable = (ROOT / "disc/SLPS_018.23").read_bytes()
    for table in manifest.get("binary_read_tables", []):
        calls = [r for r in reads if r.get("address") == table["call"]]
        require(len(calls) == 1, f"missing live assembly call for {table['symbol']}; migrate its table mapping to C")
        calls[0].update(coverage="table_driven", table=table["symbol"])
        for row in range(table["rows"]):
            address = table["address"] + row * table["stride"]
            sector, count = (struct.unpack_from("<h", executable, address - 0x80010000 + 0x800 + field)[0]
                             for field in table["fields"])
            record = {"source": "disc/SLPS_018.23", "kind": "binary_table_row", "table": table["symbol"],
                      "row": row, "address": address, "command": table["command"], "destination": table["destination"]}
            if sector == count == 0:
                record.update(coverage="absent", sector=0, sector_count=0)
            else:
                record.update(intersections(sector, count, entries))
            reads.append(record)

    def check_destination(destination, count, source):
        if not count:
            return
        if isinstance(destination, str):
            destination = destination.strip()
            if re.fullmatch(r"0x[\da-fA-F]+", destination):
                destination = int(destination, 16)
            else:
                for region in regions:
                    if destination in [region["macro"], *region.get("views", [])]:
                        destination = region["address"]
                        break
        if not isinstance(destination, int):
            return  # Dynamic slots and executable-owned buffers retain their expression.
        require(0x80000000 <= destination < destination + count * SECTOR <= 0x80200000,
                f"read exceeds RAM: {source}")
        for region in regions:
            if destination == region["address"]:
                require(count * SECTOR <= region["size"], f"read exceeds {region['symbol']}")

    for read in reads:
        destination = read.get("destination")
        match = re.fullmatch(r"\([^()]+\*\)\s*(0x[\da-fA-F]+)", destination) if isinstance(destination, str) else None
        if match:
            destination = int(match[1], 16)
        count = read.get("sector_count", 0)
        check_destination(destination, count, read["source"])
        for use in read.get("uses", []):
            if "column" not in use or read["row"] % 3 == use["column"]:
                check_destination(use["destination"], count, read["source"])
    for entry in entries:
        entry["read_indices"] = [i for i, r in enumerate(reads) if any(h["entry"] == entry["index"] for h in r.get("entries", []))]
    return reads


def generate():
    data, entries, manifest = catalogue()
    regions = runtime_symbols(manifest)
    values = constants(entries, regions)
    header = ["/* Generated by tools/snb_dat.py; edit config/dat_resources.yaml or symbol_addrs.txt. */", "#pragma once", ""]
    header += [f"#define {name} 0x{value:X}" for name, value in values.items()]
    write_text(ROOT / "build/include/game/io/dat_entries.h", "\n".join(header) + "\n")
    # Keep runtime symbols visible to the linker without allocating storage.
    write_text(OUTPUT / "runtime.ld", "/* Absolute RAM views; no storage is allocated. */\n" +
               "".join(f"{r['symbol']} = 0x{r['address']:08X};\n" for r in regions))
    config = {"name": "SNB.DAT", "sha1": DAT_SHA1, "options": {
        "basename": "snb_dat", "target_path": "disc/SNB/SNB.DAT", "base_path": "../..",
        "platform": "psx", "compiler": "PSYQ", "asset_path": "build/dat/assets",
        "asm_path": "build/dat/asm", "src_path": "build/dat/src", "build_path": "build/dat/objects",
        "cache_path": "build/dat/splat.cache", "symbol_addrs_path": [], "reloc_addrs_path": [],
        "create_undefined_funcs_auto": False, "create_undefined_syms_auto": False,
        "generate_asm_macros_files": False}, "segments": [
            {"start": 0, "type": "bin", "name": "header", "linker_entry": False},
            *[{"start": e["offset"], "type": "bin", "name": e["file"][:-4], "linker_entry": False} for e in entries],
            [len(data)]]}
    write_text(OUTPUT / "snb_dat.yaml", yaml.safe_dump(config, sort_keys=False))
    print(f"[dat] Generated Splat config, C constants and runtime symbols for {len(entries)} entries")


def map_reads():
    data, entries, manifest = catalogue()
    regions = runtime_symbols(manifest)
    reads = inventory(entries, manifest, regions)
    report = {"sha1": DAT_SHA1, "size": len(data), "sector_size": SECTOR, "entries": entries,
              "runtime_regions": regions, "reads": reads}
    # Child files are inspection aids. Raw top-level spans remain the packing
    # inputs, preserving each offset table, flag bit, gap and trailing padding.
    for entry in entries:
        if entry["format"] != "offset_archive":
            continue
        for child in entry["children"]:
            relative = f"nested/entry_{entry['index']:03d}/child_{child['index']:03d}.bin"
            start = entry["offset"] + child["offset"]
            payload = data[start:start + child["allocated_size"]]
            child["file"] = relative
            path = OUTPUT / relative
            path.parent.mkdir(parents=True, exist_ok=True)
            if not path.exists() or path.read_bytes() != payload:
                path.write_bytes(payload)
    write_text(OUTPUT / "catalogue.json", json.dumps(report, indent=2) + "\n")
    lines = ["# SNB.DAT catalogue", "", "Generated from the original DAT, source, and live assembly fallbacks.", "",
             "| Index | Name | Offset | Sectors | Format | Reads |", "| --- | --- | --- | --- | --- | --- |"]
    lines += [f"| {e['index']} | {e['name']} | 0x{e['offset']:X} | {e['sector_count']} | {e['format']} | {len(e['read_indices'])} |" for e in entries]
    lines += ["", "## Read inventory", "", "Table rows carry their command/destination uses in catalogue.json.", "",
              "| Source | Location | Coverage | Sector | Count | Entries |", "| --- | --- | --- | --- | --- | --- |"]
    for r in report["reads"]:
        location = r.get("table", "") + (f"[{r['row']}]" if "row" in r else f"0x{r['address']:X}" if "address" in r else f":{r['line']}")
        lines.append(f"| {r['source']} | {location} | {r['coverage']} | {r.get('sector', '')} | {r.get('sector_count', '')} | {', '.join(str(h['entry']) for h in r.get('entries', []))} |")
    write_text(OUTPUT / "catalogue.md", "\n".join(lines) + "\n")
    print(f"[dat] Generated {len(entries)} entries; reads: {dict(Counter(r['coverage'] for r in report['reads']))}")


def extracted_parts(entries):
    parts = [("header.bin", 0, entries[0]["offset"])]
    parts += [(e["file"], e["offset"], e["end"]) for e in entries]
    for name, start, end in parts:
        path = OUTPUT / "assets" / name
        data = path.read_bytes()
        require(len(data) == end - start, f"{path}: extracted span has wrong size")
        yield path, start, end, data


def pack(output):
    _, entries, _ = catalogue()
    require(output.resolve() != INPUT.resolve() and
            (not output.exists() or output.resolve() == (OUTPUT / "SNB.DAT").resolve()),
            "pack output must be new or build/dat/SNB.DAT; original input cannot be overwritten")
    chunks = [data for _, _, _, data in extracted_parts(entries)]
    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_bytes(b"".join(chunks))
    print(f"[dat] Packed {output.relative_to(ROOT) if output.is_relative_to(ROOT) else output}")


def verify():
    original, entries, manifest = catalogue()
    for path, start, end, data in extracted_parts(entries):
        require(data == original[start:end], f"{path}: bytes differ from original DAT")
    rebuilt = (OUTPUT / "SNB.DAT").read_bytes()
    require(rebuilt == original and hashlib.sha1(rebuilt).hexdigest() == DAT_SHA1, "rebuilt DAT mismatch")
    # Recompute rather than trusting stale reports or generated constants.
    regions = runtime_symbols(manifest)
    reads = inventory(entries, manifest, regions)
    for entry in entries:
        if entry["format"] != "offset_archive":
            continue
        for child in entry["children"]:
            relative = f"nested/entry_{entry['index']:03d}/child_{child['index']:03d}.bin"
            child["file"] = relative
            start = entry["offset"] + child["offset"]
            require((OUTPUT / relative).read_bytes() == original[start:start + child["allocated_size"]],
                    f"{relative}: nested bytes differ")
    report = json.loads((OUTPUT / "catalogue.json").read_text())
    require(report["entries"] == entries, "stale DAT catalogue; regenerate through build wrapper")
    require(report["reads"] == reads and report["runtime_regions"] == regions,
            "stale DAT references; regenerate through build wrapper")
    print(f"[dat] Verified 395 entries, header/padding, extracted bytes, runtime bounds and rebuilt SHA1 {DAT_SHA1}")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("operation", choices=("inspect", "generate", "map", "pack", "verify"))
    parser.add_argument("--output", type=Path, default=OUTPUT / "SNB.DAT", help="pack output (never the disc input)")
    args = parser.parse_args()
    try:
        if args.operation == "generate":
            generate()
        elif args.operation == "map":
            map_reads()
        elif args.operation == "pack":
            pack(args.output)
        elif args.operation == "verify":
            verify()
        else:
            _, entries, manifest = catalogue()
            regions = runtime_symbols(manifest)
            reads = inventory(entries, manifest, regions)
            print(json.dumps({"entries": entries, "runtime_regions": regions, "reads": reads}, indent=2))
    except (OSError, ValueError, KeyError, struct.error, yaml.YAMLError) as error:
        parser.exit(1, f"[dat] {error}\n")


if __name__ == "__main__":
    main()
