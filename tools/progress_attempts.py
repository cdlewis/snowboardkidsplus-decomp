"""Validate structured C attempt annotations beside live assembly fallbacks."""

import re
import sys
from dataclasses import dataclass
from fractions import Fraction

from decomp import Elf32, INCLUDE, ROOT, parse_asm, uncomment

MARKER = re.compile(r"/\* DECOMP_ATTEMPT: (unattempted|(?:object|inline) (?:0|[1-9][0-9]*)/[1-9][0-9]*) \*/")
INLINE_START = "#ifdef NON_MATCHING"
DIRECTIVE = re.compile(r"^\s*#\s*(if|ifdef|ifndef|else|elif|endif)\b", re.M)


@dataclass(frozen=True)
class Attempt:
    kind: str
    score: Fraction | None
    line: int


def has_definition(text, name):
    return re.search(r"\b" + re.escape(name) + r"\s*\([^;{}]*\)\s*\{", uncomment(text)) is not None


def annotation(lines, line, name):
    """Find the preceding annotation and confirm the inline guard, when present."""
    if line and lines[line - 1].strip() == "#else":
        end = line - 1
        depth = 0
        for start in range(end - 1, -1, -1):
            directive = DIRECTIVE.match(lines[start])
            if directive is None:
                continue
            keyword = directive[1]
            if keyword == "endif":
                depth += 1
            elif keyword in {"if", "ifdef", "ifndef"}:
                if depth == 0:
                    break
                depth -= 1
        else:
            raise ValueError("missing #ifdef NON_MATCHING for guarded INCLUDE_ASM")
        if (start == 0 or lines[start].strip() != INLINE_START
                or line + 1 == len(lines) or lines[line + 1].strip() != "#endif"
                or not has_definition("\n".join(lines[start + 1:end]), name)):
            raise ValueError("expected #ifdef NON_MATCHING with a C definition and an assembly #else")
        return lines[start - 1].strip(), True, start - 1
    return lines[line - 1].strip() if line else "", False, line - 1


def inline_source(text, name):
    """Return a scored candidate with the declarations in its guard."""
    lines = text.splitlines()
    stripped = uncomment(text)
    for fallback in INCLUDE.finditer(stripped):
        if fallback[2] == name:
            line = stripped.count("\n", 0, fallback.start())
            _, inline, marker_line = annotation(lines, line, name)
            if inline:
                return "\n".join(lines[marker_line + 2:line - 1]) + "\n"
            break
    return None


def read_attempts(root):
    attempts = {}
    for source in sorted((root / "src").rglob("*.c")):
        text = source.read_text()
        lines = text.splitlines()
        stripped = uncomment(text)
        used_annotations = set()
        for fallback in INCLUDE.finditer(stripped):
            line = stripped.count("\n", 0, fallback.start())
            name = fallback[2]
            try:
                raw, inline, marker_line = annotation(lines, line, name)
                match = MARKER.fullmatch(raw)
                if match is None:
                    raise ValueError("missing or invalid DECOMP_ATTEMPT annotation")
                if marker_line in used_annotations:
                    raise ValueError("DECOMP_ATTEMPT annotation covers multiple fallbacks")
                used_annotations.add(marker_line)
                kind, _, words = match[1].partition(" ")
                if kind == "unattempted":
                    if inline:
                        raise ValueError("inline comparison C requires measured word counts")
                    attempts[source.relative_to(root), name] = Attempt(kind, None, marker_line + 1)
                    continue
                if not inline:
                    raise ValueError("measured attempts require inline C under #ifdef NON_MATCHING")
                matched, total = map(int, words.split("/"))
                asm = root / fallback[1] / (name + ".s")
                target_words = len(parse_asm(asm).data) // 4
                if matched > target_words or total < target_words:
                    raise ValueError(f"invalid {words} comparison for a {target_words}-word target")
                attempts[source.relative_to(root), name] = Attempt(kind, Fraction(matched, total), marker_line + 1)
            except ValueError as error:
                raise ValueError(f"{source.relative_to(root)}:{line + 1}: {name}: {error}") from error

        for index, line in enumerate(lines):
            if line.lstrip().startswith("/* DECOMP_ATTEMPT:") and index not in used_annotations:
                raise ValueError(f"{source.relative_to(root)}:{index + 1}: orphaned DECOMP_ATTEMPT annotation")
    return attempts


def verify_comparison_objects(root):
    units = {}
    for (unit, name), attempt in read_attempts(root).items():
        if attempt.score is None:
            continue
        if unit not in units:
            units[unit] = Elf32(root / "build/compare" / unit.with_suffix(".o"))
        symbol = units[unit].symbols.get(name)
        if not symbol or symbol.kind != 2 or not symbol.section or not symbol.size:
            raise ValueError(f"{unit}: comparison object is missing the annotated C definition of {name}")


if __name__ == "__main__":
    try:
        attempts = read_attempts(ROOT)
    except (ValueError, OSError) as error:
        sys.exit(str(error))
    print(f"Validated {len(attempts)} decompilation attempt annotations.")
