#!/usr/bin/env python3
"""Prepare pinned Psy-Q 4.2 objects and its bundled 4.2.1 library patches.

Run `python3 tools/psyq_sdk.py --fetch` once, then
`./tools/build-and-verify.sh`. SDK originals come from the pinned submodule;
the downloaded converter and generated objects remain ignored.
The build archives the converted objects; Splat selects their sections in YAML.
Fully matched C remains active alongside the imported SDK members.
The verifier checks linked section placement, initialized bytes, BSS, exports, and imports.
"""

import argparse
import hashlib
import io
import re
from pathlib import Path
import struct
import subprocess
import tarfile
import urllib.request

from decomp import Elf32, configured_symbols

ROOT = Path(__file__).resolve().parent.parent
CONVERTER = {
    'url': 'https://github.com/decompme/compilers/releases/download/compilers/psyq-obj-parser.tar.gz',
    'archive_sha256': '353495f13f6756773cd905ba3a0743843232f95719e306fc6ee56cbf7d731a3e',
    'sha256': '4fba623a31bb5830ceea95b5081bc13504629686b0b4904c533b7be2a5f2377a',
}

CACHE = ROOT / "tools/psyq-sdk"


def sdk_config(root=ROOT):
    """Read SDK selection and placement from Splat, hashes from SDK provenance."""
    import json
    import yaml

    layout = yaml.safe_load((root / "snowboardkidsplus.yaml").read_text())
    objects = {}
    for segment in layout["segments"]:
        if not isinstance(segment, dict):
            continue
        for entry in segment.get("subsegments", []):
            if isinstance(entry, list):
                if len(entry) < 5 or entry[1] != "lib":
                    continue
                start, _, library, member, section = entry[:5]
            else:
                if entry.get("type") != "lib":
                    continue
                start, library, member, section = (entry[k] for k in ("start", "name", "object", "section"))
            key = library + "/" + member
            spec = objects.setdefault(key, {"library": library.upper() + ".LIB",
                                            "member": member.upper(), "sections": {}})
            address = segment["vram"] + start - segment["start"]
            spec["sections"][section] = {"address": hex(address),
                                         "noload": section.startswith((".bss", ".sbss"))}
    paths = {"LIBPAD421.LIB": "4.2.1/libpad/LIBPAD.LIB",
             "LIBMCRD421.LIB": "4.2.1/libmcrd/LIBMCRD.LIB"}
    provenance = json.loads((root / "lib/psyq-sdk/PROVENANCE.json").read_text())
    libraries = {spec["library"]: provenance["sha256"][paths.get(spec["library"], "4.2/LIB/" + spec["library"])]
                 for spec in objects.values()}
    return {"sdk": {"path": "lib/psyq-sdk", "library_paths": paths},
            "libraries": libraries, "objects": objects}


def split_commons(path, dump):
    """Restore independent storage for Psy-Q UNDEFN/common declarations.

    The pinned older converter appends these as weak symbols to .bss. They
    are shared commons, not one contiguous object-local allocation. Newer
    upstream supports preserving this distinction with -c:
    https://github.com/grumpycoders/pcsx-redux/blob/dd04dfdb0795fd25dbc3bec9691c00d495110a5b/tools/psyq-obj-parser/psyq-obj-parser.cc

    Give each common its own NOBITS section so Splat can place it explicitly.
    Rebase any REL addends that the older converter folded into .bss offsets.
    Instruction opcodes and all non-relocated initialized bytes stay intact.
    """
    original = Elf32(path)
    sections = [list(section) for section in original.sections]
    strings_index = struct.unpack_from("<H", original.data, 50)[0]
    names = bytearray(original.section_data(strings_index))
    data = bytearray(original.data)
    localsizes = {}
    for line in dump.splitlines():
        match = re.match(r"\s*[\da-f]{4}\s+[\da-f]{4}\s+\d+\s+[\da-f]{8}\s+([\da-f]{8})\s+([\da-f]{8})\s+[\da-f]{8}\s+(\.\w+)\s*$", line)
        if match:
            localsizes[match[3]] = int(match[1], 16) + int(match[2], 16)
    touched = set()
    common_symbols = {}
    permitted_changes = set()
    for section in original.sections:
        if section[1] != 2:
            continue
        strings = original.section_data(section[6])
        for offset in range(section[4], section[4] + section[5], section[9]):
            name, value, size, info, other, index = struct.unpack_from("<IIIBBH", data, offset)
            if info >> 4 != 2 or not 0 < index < len(original.sections):
                continue
            if original.sections[index][1] != 8:
                continue  # Weak functions are not common-storage declarations.
            section_name = original.section_names[index]
            if section_name not in localsizes:
                raise ValueError(f"missing original local storage size for {section_name}")
            touched.add(index)
            symbol_name = original.string(strings, name)
            symbol_index = (offset - section[4]) // section[9]
            common_symbols[symbol_name] = symbol_index
            new_name = ".bss.common_" + symbol_name
            name_offset = len(names)
            names.extend(new_name.encode("ascii") + b"\0")
            alignment = 8 if size >= 8 else 4 if size >= 4 else 2 if size >= 2 else 1
            new_index = len(sections)
            sections.append([name_offset, 8, 3, 0, 0, size, 0, 0, alignment, 0])
            struct.pack_into("<IIIBBH", data, offset, name, 0, size, 0x10 | (info & 15), other, new_index)
    for index in touched:
        sections[index][5] = localsizes[original.section_names[index]]
    if not touched:
        return
    # The converter lowers common+addend expressions to section-relative RELs.
    # Recover their named symbol and original addend from its LNK dump.
    expressions = {}
    for line in dump.splitlines():
        match = re.match(r"\s*(HI16|LO16|REL32|REL26)\s+(\.\w+)::([\da-f]{8})\s+(\w+)(?:\s*\+\s*(-?\d+))?\s*$", line)
        if match and match[4] in common_symbols:
            expressions[(match[2], int(match[3], 16))] = (common_symbols[match[4]], int(match[5] or 0))
    for section in original.sections:
        if section[1] != 9:
            continue
        source_index = section[7]
        source = original.sections[source_index]
        for position in range(section[4], section[4] + section[5], 8):
            offset, info = struct.unpack_from("<II", data, position)
            expression = expressions.get((original.section_names[source_index], offset))
            if expression is None:
                continue
            symbol_index, addend = expression
            kind = info & 255
            struct.pack_into("<I", data, position + 4, (symbol_index << 8) | kind)
            value = source[4] + offset
            if kind in (5, 6):
                immediate = ((addend + 0x8000) >> 16) if kind == 5 else addend
                struct.pack_into("<H", data, value, immediate & 0xFFFF)
                permitted_changes.update(range(value, value + 2))
            elif kind == 2:
                struct.pack_into("<I", data, value, addend & 0xFFFFFFFF)
                permitted_changes.update(range(value, value + 4))
            else:
                raise ValueError(f"unsupported common relocation kind {kind}")
    sections[strings_index][4:6] = [len(data), len(names)]
    data.extend(names)
    data.extend(b"\0" * (-len(data) % 4))
    table_offset = len(data)
    for section in sections:
        data.extend(struct.pack("<10I", *section))
    struct.pack_into("<I", data, 32, table_offset)
    struct.pack_into("<H", data, 48, len(sections))
    path.write_bytes(data)
    converted = Elf32(path)
    for index, section in enumerate(original.sections):
        if section[1] == 1:
            before, after = original.section_data(index), converted.section_data(index)
            if len(before) != len(after) or any(a != b and section[4] + offset not in permitted_changes
                                             for offset, (a, b) in enumerate(zip(before, after))):
                raise ValueError("common conversion modified non-relocated initialized bytes")


def verify(data, expected, description):
    actual = hashlib.sha256(data).hexdigest()
    if actual != expected:
        raise ValueError(f"{description}: SHA256 {actual}, expected {expected}")
    return data


def checked_file(path, expected):
    if not path.is_file():
        raise ValueError(f"missing {path}; run python3 tools/psyq_sdk.py --fetch")
    return verify(path.read_bytes(), expected, str(path))


def download(url, path, expected):
    if path.exists():
        return checked_file(path, expected)
    print(f"[sdk42] Downloading {url}", flush=True)
    with urllib.request.urlopen(url, timeout=60) as response:
        data = verify(response.read(), expected, url)
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_bytes(data)
    return data


def fetch(config):
    converter = CONVERTER
    data = download(converter["url"], CACHE / "psyq-obj-parser.tar.gz",
                    converter["archive_sha256"])
    with tarfile.open(fileobj=io.BytesIO(data), mode="r:gz") as archive_file:
        member = archive_file.getmember("psyq-obj-parser")
        if not member.isfile():
            raise ValueError("converter archive member is not a regular file")
        with archive_file.extractfile(member) as stream:
            binary = verify(stream.read(), converter["sha256"], "psyq-obj-parser")
    path = CACHE / "psyq-obj-parser"
    path.write_bytes(binary)
    path.chmod(0o755)
    for name, checksum in config["libraries"].items():
        checked_file(library_path(config, name), checksum)
    print("[sdk42] Verified SDK libraries and converter.", flush=True)


def library_path(config, name):
    relative = config["sdk"].get("library_paths", {}).get(name, "4.2/LIB/" + name)
    return ROOT / config["sdk"]["path"] / relative


def extract_member(data, wanted):
    # LIB v1: magic, then members with name[8], timestamp, LNK offset,
    # and member length. Offsets and lengths are relative to the member header.
    if data[:4] != b"LIB\x01":
        raise ValueError("expected a Psy-Q LIB v1 archive")
    position = 4
    found = None
    while position < len(data):
        if position + 20 > len(data):
            raise ValueError("truncated library member header")
        name = data[position:position + 8].rstrip(b" \0").decode("ascii")
        offset, size = struct.unpack_from("<II", data, position + 12)
        if not 20 <= offset < size or position + size > len(data):
            raise ValueError(f"invalid library member bounds: {name}")
        member = data[position + offset:position + size]
        if member[:4] != b"LNK\x02":
            raise ValueError(f"expected LNK v2 object: {name}")
        if name == wanted:
            if found is not None:
                raise ValueError(f"duplicate library member: {wanted}")
            found = member
        position += size
    if found is None:
        raise ValueError(f"library member not found: {wanted}")
    return found


def prepare(config, object_name, output, objcopy):
    spec = config["objects"][object_name]
    library = checked_file(library_path(config, spec["library"]),
                           config["libraries"][spec["library"]])
    converter = CACHE / "psyq-obj-parser"
    checked_file(converter, CONVERTER["sha256"])
    member = extract_member(library, spec["member"])
    output.parent.mkdir(parents=True, exist_ok=True)
    original = output.with_suffix(".sdk.obj")
    converted = output.with_suffix(".sdk.o")
    original.write_bytes(member)
    with output.with_suffix(".sdk.log").open("w") as log:
        subprocess.run([str(converter), str(original), "-d", "-o", str(converted)],
                       stdout=log, stderr=subprocess.STDOUT, check=True)
    split_commons(converted, output.with_suffix(".sdk.log").read_text())
    elf = Elf32(converted)
    allocated = {name: section[5] for name, section in zip(elf.section_names, elf.sections)
                 if section[2] & 2 and section[5]}
    discarded = set(allocated) - set(spec["sections"])
    if set(spec["sections"]) - set(allocated):
        raise ValueError(f"unexpected SDK sections for {object_name}: {allocated}")
    command = [objcopy]
    for section in discarded:
        # objcopy also rejects removal if a surviving relocation needs it.
        if not section.startswith(".bss.common_"):
            raise ValueError(f"cannot discard initialized SDK section {section}")
        command += ["--remove-section", section]
    for name, symbol in configured_symbols().items():
        for section_name, section in spec["sections"].items():
            address, size = int(section["address"], 0), allocated[section_name]
            if not address <= symbol.address < address + size:
                continue
            offset = symbol.address - address
            if name not in elf.symbols:
                flags = "global,function" if section_name == ".text" else "global"
                command += ["--add-symbol", f"{name}={section_name}:0x{offset:X},{flags}"]
    for section in spec["sections"]:
        # Local metadata gives the verifier a section address without adding
        # artificial function boundaries to the map-based progress report.
        sentinel = section_symbol(object_name, section)
        command += ["--add-symbol", f"{sentinel}={section}:0,local"]
    command += [str(converted), str(output)]
    subprocess.run(command, check=True)
    print(f"[sdk42] {spec['library']}/{spec['member']} -> {output}", flush=True)


def section_symbol(object_name, section):
    return "__sdk42_" + object_name.replace("/", "_") + "_" + section.lstrip(".")


def verify_linked(config):
    from decomp import PsxImage
    elf = Elf32(ROOT / "build/slps_018.23.elf")
    image = PsxImage((ROOT / "disc/SLPS_018.23").read_bytes())
    count = 0
    symbols_checked = set()
    configured = configured_symbols()
    for object_name, spec in config["objects"].items():
        source = Elf32(ROOT / "build/sdk/psyq42" / (object_name + ".o"))
        allocated = {name: section[5] for name, section in zip(source.section_names, source.sections)}
        for section_name, section in spec["sections"].items():
            marker = elf.symbols.get(section_symbol(object_name, section_name))
            address, size = int(section["address"], 0), allocated[section_name]
            if marker is None or marker.address != address:
                raise ValueError(f"{object_name}{section_name}: wrong linked address; expected 0x{address:X}")
            containing = elf.sections[marker.section]
            if address + size > containing[3] + containing[5]:
                raise ValueError(f"{object_name}{section_name}: linked section is too short")
            if section.get("noload"):
                if containing[1] != 8:
                    raise ValueError(f"{object_name}{section_name}: expected NOBITS storage")
            else:
                offset = address - containing[3]
                actual = elf.section_data(marker.section)[offset:offset + size]
                if actual != image.read(address, size):
                    raise ValueError(f"{object_name}{section_name}: initialized bytes differ")
            count += 1
        for table in source.sections:
            if table[1] != 2:
                continue
            strings = source.section_data(table[6])
            for offset in range(table[4], table[4] + table[5], table[9]):
                name, value, _, info, _, index = struct.unpack_from("<IIIBBH", source.data, offset)
                if not info >> 4 or index >= len(source.sections):
                    continue
                name = source.string(strings, name)
                if not name or name.endswith(".NON_MATCHING"):
                    continue
                section_name = source.section_names[index]
                if index == 0:
                    if name not in configured:
                        raise ValueError(f"{object_name}: SDK import {name} has no canonical address")
                    expected = configured[name].address
                elif section_name in spec["sections"]:
                    expected = int(spec["sections"][section_name]["address"], 0) + value
                else:
                    continue
                actual = elf.symbols.get(name)
                if actual is None or actual.address != expected:
                    raise ValueError(f"{object_name}: wrong linked symbol {name}; expected 0x{expected:X}")
                symbols_checked.add(name)
    print(f"[sdk42] Verified {len(config['objects'])} objects, {count} section placements, "
          f"and {len(symbols_checked)} symbol bindings, including BSS.", flush=True)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--fetch", action="store_true")
    parser.add_argument("--list-objects", action="store_true")
    parser.add_argument("--list-library-inputs", action="store_true")
    parser.add_argument("--verify", action="store_true", help="verify linked SDK storage and bytes")
    parser.add_argument("--object")
    parser.add_argument("--output", type=Path)
    parser.add_argument("--objcopy", default="mips-linux-gnu-objcopy")
    args = parser.parse_args()
    if not any((args.fetch, args.object, args.list_objects, args.list_library_inputs,
                args.verify)):
        parser.error("specify --fetch, --object, --list-objects, or --verify")
    if args.object and args.output is None:
        parser.error("--object requires --output")
    if args.fetch:
        subprocess.run(["git", "submodule", "update", "--init", "--", "lib/psyq-sdk"],
                       cwd=ROOT, check=True)
    config = sdk_config()
    try:
        if args.list_objects:
            print(" ".join(config["objects"]))
        if args.list_library_inputs:
            print(" ".join(library_path(config, name).relative_to(ROOT).as_posix()
                           for name in config["libraries"]))
        if args.verify:
            verify_linked(config)
        if args.fetch:
            fetch(config)
        if args.object:
            if args.object not in config["objects"]:
                raise ValueError(f"object is not enabled for SDK linkage: {args.object}")
            prepare(config, args.object, args.output, args.objcopy)
    except (OSError, ValueError, subprocess.CalledProcessError) as error:
        parser.exit(1, f"[sdk42] {error}\n")


if __name__ == "__main__":
    main()
