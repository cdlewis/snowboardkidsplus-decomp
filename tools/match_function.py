#!/usr/bin/env python3
"""Compile an isolated C attempt with the owning unit's real PSX pipeline.

Uses real project headers and resolves relocations against a verified image.
Artifacts stay in nonmatchings/, never build/ or expected/. This is a byte
check, not a fuzzy scorer; use objdiff for ranking near misses.
"""

import argparse
import difflib
import hashlib
import json
import re
import subprocess
import sys
from pathlib import Path

import build
from decomp import (ROOT, Elf32, PsxImage, active_functions, configured_symbols,
                    first_difference, require_fresh, uncomment)


def owning_unit(name, functions):
    active = next((f for f in functions if f.name == name), None)
    if active:
        return active.unit
    pattern = re.compile(r'\b' + re.escape(name) + r'\s*\([^;{}]*\)\s*\{')
    found = [p.relative_to(ROOT) for p in (ROOT / "src").rglob("*.c")
             if pattern.search(uncomment(p.read_text()))]
    if len(found) != 1:
        raise ValueError("cannot identify one owning C unit; specify --unit src/path.c")
    return found[0]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("function")
    parser.add_argument("source", type=Path, help="one function body plus headers and extern declarations")
    parser.add_argument("--unit", type=Path, help="owning unit, for its compiler settings")
    parser.add_argument("--output", type=Path, help="isolated artifact directory")
    parser.add_argument("--cc-flag", action="append", default=[], help="candidate-only experiment; use --cc-flag=-O1")
    args = parser.parse_args()
    require_fresh()
    target_path = ROOT / "disc/SLPS_018.23"
    original = PsxImage(target_path.read_bytes())
    if original.data != (ROOT / "build/slps_018.23.exe").read_bytes():
        raise ValueError("the live full-image gate is red; restore it before isolated experiments")
    reference = Elf32(ROOT / "build/slps_018.23.elf")
    functions = list(active_functions(sdk=True))
    oracle = next((f for f in functions if f.name == args.function), None)
    unit = args.unit or owning_unit(args.function, functions)
    if unit.is_absolute() or ".." in unit.parts or not (ROOT / unit).is_file():
        raise ValueError("--unit must identify an existing repository-relative C source")
    spec = configured_symbols().get(args.function)
    linked = reference.symbols.get(args.function)
    address = oracle.address if oracle else spec.address if spec else None
    size = len(oracle.data) if oracle else spec.size if spec and spec.size else linked.size if linked else 0
    if address is None or not size:
        raise ValueError("original function extent unknown; split the function or add size metadata")
    target = original.read(address, size)
    if oracle and target != oracle.data:
        raise ValueError("assembly oracle disagrees with the original executable; run make split")
    if any(f.startswith("-G") for f in args.cc_flag):
        raise ValueError("-G must agree across cc1, maspsx and as; use the owning unit's setting")
    source = args.source.resolve()
    output = (args.output or ROOT / "nonmatchings" / args.function / source.stem).resolve()
    if output.is_relative_to(ROOT) and not output.is_relative_to(ROOT / "nonmatchings"):
        raise ValueError("repository scratch output must stay inside nonmatchings/")
    output.mkdir(parents=True, exist_ok=True)
    obj = build.compile_c(source, True, output_root=output, unit_path=unit,
                          only_function=args.function, extra_flags=args.cc_flag)
    candidate = Elf32(obj)
    symbol = candidate.symbols.get(args.function)
    if not symbol or symbol.kind != 2 or not symbol.size:
        raise ValueError("candidate must define the requested function")
    definitions = [s.name for s in candidate.symbols.values()
                   if s.kind == 2 and s.section and s.name != args.function]
    data_sections = {".data", ".rodata", ".rdata", ".sdata", ".bss", ".sbss"}
    if definitions or any(section[5] for name, section in zip(candidate.section_names, candidate.sections)
                          if name in data_sections):
        raise ValueError("isolate one function and declare other objects extern; use full-TU objdiff for owned data/jump tables")
    assignments = {"_gp": reference.symbols["_gp"].address}
    for symbol in candidate.symbols.values():
        if symbol.section == 0 and symbol.name != args.function:
            expected = reference.symbols.get(symbol.name)
            if expected is None:
                raise ValueError(f"unresolved external {symbol.name}; use canonical project declarations")
            assignments[symbol.name] = expected.address
    script = output / "candidate.ld"
    script.write_text(f"SECTIONS {{ .text 0x{address:X} : SUBALIGN(4) {{ *(.text) }} "
                      "/DISCARD/ : { *(.reginfo) *(.MIPS.abiflags) *(.pdr) *(.comment) *(.gnu.attributes) } }\n" +
                      "\n".join(f"{name} = 0x{value:X};" for name, value in assignments.items()))
    elf = output / "candidate.elf"
    subprocess.run([build.LD, "-EL", "-T", str(script), "-o", str(elf), str(obj)], check=True)
    final = Elf32(elf)
    data = final.read_symbol(args.function)
    (output / "target.bin").write_bytes(target)
    (output / "candidate.bin").write_bytes(data)
    for label in ("target", "candidate"):
        result = subprocess.run([build.CROSS + "objdump", "-D", "-b", "binary", "-m", "mips:3000", "-EL",
                                 f"--adjust-vma={address}", str(output / (label + ".bin"))],
                                check=True, text=True, capture_output=True)
        (output / (label + ".dis")).write_text(result.stdout)
    difference = "".join(difflib.unified_diff(
        (output / "target.dis").read_text().splitlines(True)[3:],
        (output / "candidate.dis").read_text().splitlines(True)[3:],
        fromfile="target", tofile="candidate"))
    (output / "diff.txt").write_text(difference)
    equal = data == target and final.symbols[args.function].address == address
    settings = build.PER_TU_FLAGS.get(unit.as_posix(), {})
    report = {"function": args.function, "unit": str(unit), "source": str(source),
              "compiler": str(settings.get("cc1", build.CC1)),
              "g_value": settings.get("g_value", build.G_VALUE),
              "unit_cc_flags": settings.get("cc1_flags", []),
              "function_flags": settings.get("function_flags", {}).get(args.function, []),
              "extra_flags": args.cc_flag, "target_sha1": hashlib.sha1(original.data).hexdigest(),
              "source_sha1": hashlib.sha1(source.read_bytes()).hexdigest(),
              "preprocessed_sha1": hashlib.sha1(obj.with_suffix(".i").read_bytes()).hexdigest(),
              "aspsx_version": build.ASPSX_VERSION,
              "gas_reorder": settings.get("gas_reorder", False),
              "pad_sections": settings.get("pad_sections", False),
              "address": address, "target_size": len(target), "candidate_size": len(data),
              "exact": equal, "first_byte_difference": first_difference(target, data)}
    (output / "result.json").write_text(json.dumps(report, indent=2) + "\n")
    print(f"{args.function}: {'EXACT' if equal else 'DIFFERENT'}; "
          f"{len(data)} candidate bytes / {len(target)} target bytes")
    print(f"Artifacts: {output}")
    return 0 if equal else 1


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (ValueError, OSError, KeyError, subprocess.CalledProcessError) as error:
        print(error, file=sys.stderr)
        raise SystemExit(2)
