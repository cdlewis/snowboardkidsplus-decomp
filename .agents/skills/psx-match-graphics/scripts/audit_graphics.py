#!/usr/bin/env python3
"""Read-only C source triage for likely PSX graphics macro operations.

This is a lexical heuristic, not a C parser, preprocessor, or dataflow proof.
Aliases are function-local and flow-insensitive; indirect calls, included macro
bodies, and arbitrary pointer arithmetic still require human inspection.
"""

import argparse
import bisect
import json
from pathlib import Path
import re


TOKEN = re.compile(r"\b[A-Za-z_]\w*\b")
LEXEME = re.compile(
    r'//[^\n]*|/\*[\s\S]*?\*/|"(?:\\[\s\S]|[^"\\])*"|\'(?:\\[\s\S]|[^\'\\])*\'')
PACKET_TYPE = r"(?:PACKET|P_TAG|POLY_\w+|SPRT(?:_\w+)?|TILE(?:_\w+)?|LINE_\w+|DR_\w+|\w*Packet)"
PACKET_POINTER = re.compile(r"\b" + PACKET_TYPE + r"\s*\*+\s*(\w+)")
ASSIGNMENT = re.compile(r"(?<![\w.>])([A-Za-z_]\w*)\s*=(?!=)\s*([^;{}]+)")
CALL = re.compile(r"\b([A-Za-z_]\w*)\s*\(")
SDK = re.compile(
    r"^(?:gte_\w+|Gs\w+|set(?:Poly\w+|Sprt\w*|Tile\w*|Line\w*|RGB[0-3]|"
    r"XY\w*|UV\w*|WH|RECT|Vector|SemiTrans|ShadeTex|Draw\w+|TexWindow|"
    r"TPage|Clut|len|addr|code)|get(?:ScratchAddr|TPage|Clut|len|addr|code)|"
    r"(?:copy|add|apply)Vector|addPrims?|catPrim|termPrim|nextPrim|isendprim|"
    r"RotTransPers\w*|RotTrans|(?:Mul|Comp|Apply|Read|Rot|Trans|Scale)Matrix\w*|"
    r"Set(?:Rot|Trans|Light|Color)Matrix|NormalColor\w*|DpqColor\w*|AverageZ[34]|NormalClip)$")
SCRATCH_NAME = re.compile(r"^(?:scratch\w*|\w*SCRATCHPAD\w*)$")


def blank(match):
    return re.sub(r"[^\n]", " ", match[0])


def mask_source(source):
    """Preserve offsets and line numbers while hiding comments/literals/macros."""
    clean = LEXEME.sub(blank, source)
    return re.sub(r"^[ \t]*#(?:[^\n]*\\\n)*[^\n]*", blank, clean, flags=re.M)


def regions(clean):
    """Find top-level function bodies; keep other text in separate regions."""
    result = []
    depth, boundary, start, name = 0, 0, 0, None
    for pos, char in enumerate(clean):
        if char == "{":
            if depth == 0:
                match = re.search(r"\b(\w+)\s*\([^;{}]*\)\s*$", clean[boundary:pos])
                name = match[1] if match else None
                start = pos + 1
                if name:
                    result.append(("<file>", boundary, pos))
            depth += 1
        elif char == "}":
            depth -= 1
            if depth == 0:
                if name:
                    result.append((name, start, pos))
                else:
                    result.append(("<file>", boundary, pos + 1))
                boundary = pos + 1
                name = None
        elif char == ";" and depth == 0:
            result.append(("<file>", boundary, pos + 1))
            boundary = pos + 1
    result.append(("<file>", boundary, len(clean)))
    return result


def aliases(text, roots):
    known = set(roots)
    known.update(PACKET_POINTER.findall(text))
    known.update(name for name in TOKEN.findall(text) if SCRATCH_NAME.fullmatch(name))
    # A fixed point handles chained declarations and simple aliases, including
    # multiline assignments. It deliberately does not claim statement ordering.
    assignments = list(ASSIGNMENT.finditer(text))
    while True:
        added = {m[1] for m in assignments if known.intersection(TOKEN.findall(m[2]))}
        if added <= known:
            return known
        known.update(added)


def hints(text):
    result = []
    fields = set(re.findall(r"(?:->|\.)\s*(\w+)", text))
    if fields.intersection({f"{c}{i}" for c in "rgb" for i in range(4)}):
        result.append("LIBGPU.H: setRGB*; inspect packed reference variants")
    if fields.intersection({f"{c}{i}" for c in "xy" for i in range(4)}):
        result.append("LIBGPU.H: setXY*, setXYWH")
    if fields.intersection({f"{c}{i}" for c in "uv" for i in range(4)}):
        result.append("LIBGPU.H: setUV*, setUVWH")
    if fields.intersection({"w", "h"}):
        result.append("LIBGPU.H: setWH, setRECT")
    if fields.intersection({"tag", "len", "addr", "code"}):
        result.append("LIBGPU.H: primitive initializers, flags, tag/link helpers")
    if fields.intersection({"tpage", "clut"}):
        result.append("LIBGPU.H: getTPage/getClut vs uppercase APIs and setters")
    if fields.intersection({"vx", "vy", "vz"}):
        result.append("LIBGPU.H: vector setters; LIBGTE/INLINE_C/GTEMAC: geometry")
    if re.search(r"\bgte_\w+|\b(?:lwc2|swc2|mfc2|mtc2|cfc2|ctc2)\b", text):
        result.append("INLINE_C.H, INLINE_O.H, GTEMAC.H: leaf/composite GTE operations")
    if re.search(r"(?<![\w.>])\w+\s*(?:\+\+|--|[+-]=)", text):
        result.append("LIBGS.H: buffer cursor type/stride; distinguish DMA links")
    return result or ["Inspect SDK address, vector, packet, and GTE definitions"]


def audit(path, extra_roots):
    source = path.read_text()
    clean = mask_source(source)
    lines = [-1] + [m.start() for m in re.finditer("\n", source)]
    roots = set(extra_roots) | {"scratchpad", "getScratchAddr", "GsOUT_PACKET_P"}
    # Track locally defined view macros without expanding or executing them.
    logical = LEXEME.sub(blank, source).replace("\\\n", " ")
    definitions = "\n".join(re.findall(r"^[ \t]*#define\s+[^\n]*", logical, re.M))
    for _ in range(len(definitions.splitlines()) + 1):
        for match in re.finditer(r"#define\s+(\w+)([^\n]*)", definitions):
            if roots.intersection(TOKEN.findall(match[2])):
                roots.add(match[1])
    result = []
    for function, start, end in regions(clean):
        body = clean[start:end]
        known = aliases(body, roots)
        for statement in re.finditer(r"[^;{}]+[;{}]?", body):
            text = statement[0]
            raw = source[start + statement.start():start + statement.end()]
            raw_gte = "__asm__" in text and re.search(
                r"\b(?:lwc2|swc2|mfc2|mtc2|cfc2|ctc2|rtps|rtpt|nclip|mvmva)\b", raw)
            calls = sorted({name for name in CALL.findall(text) if SDK.fullmatch(name)})
            used = sorted(known.intersection(TOKEN.findall(text)))
            if not used and not calls and not raw_gte:
                continue
            offset = start + statement.start() + len(text) - len(text.lstrip())
            last = start + statement.start() + len(text.rstrip()) - 1
            if last < offset:
                continue
            result.append({
                "file": str(path), "function": function,
                "line": bisect.bisect_left(lines, offset),
                "end_line": bisect.bisect_left(lines, last),
                "aliases": used, "macro_or_api_calls": calls,
                "status": "macro/API present; audit coverage" if calls else "macro candidate",
                "hints": hints(text + " " + raw if raw_gte else text),
                "source": " ".join(source[offset:last + 1].split())[:200],
            })
    return result


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("paths", nargs="+", type=Path, help="C/header files or source directories")
    parser.add_argument("--root", action="append", default=[], help="additional scratchpad/packet alias root")
    parser.add_argument("--json", action="store_true", help="emit structured findings")
    args = parser.parse_args()
    files = set()
    for path in args.paths:
        if path.is_dir():
            files.update(p for p in path.rglob("*") if p.suffix in {".c", ".h"})
        elif path.is_file():
            files.add(path)
        else:
            parser.error(f"not a file or directory: {path}")
    findings = [item for path in sorted(files) for item in audit(path, args.root)]
    if args.json:
        print(json.dumps(findings, indent=2))
    else:
        print("Heuristic review candidates; no source-origin proof or automatic rewrites.")
        for item in findings:
            location = f"{item['file']}:{item['line']}"
            if item["end_line"] != item["line"]:
                location += f"-{item['end_line']}"
            print(f"{location} [{item['function']}] {item['status']}")
            print(f"  {item['source']}")
            print("  " + "; ".join(item["hints"]))
        print(f"{len(findings)} candidate statements in {len(files)} files.")


if __name__ == "__main__":
    main()
