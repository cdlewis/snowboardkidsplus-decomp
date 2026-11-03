#!/usr/bin/env python3
"""Prioritise unmatched functions with close, source-backed SDK references.

Scores compare normalised instruction sequences, not linked bytes. A reference
must have both a generated matching disassembly and a C definition. No symbols,
source files, or match status are changed by this tool.
"""

import argparse
from collections import Counter
from dataclasses import dataclass
from difflib import SequenceMatcher
from pathlib import Path
import re

from decomp import (ROOT, INCLUDE, WORD, AsmFunction, PsxImage, active_functions,
                    parse_asm, require_fresh, uncomment)
from find_similar_functions import normalize_instruction


@dataclass
class Reference:
    function: AsmFunction
    source: Path
    line: int
    tokens: tuple
    counts: Counter


def instruction_tokens(function):
    """Ignore section padding after endlabel; keep the actual return delay slot."""
    body = re.split(r"^end(?:d)?label\s+" + re.escape(function.name) + r"\b",
                    function.path.read_text(), maxsplit=1, flags=re.M)[0]
    words = sum(1 for line in body.splitlines() if WORD.search(line))
    return tuple(normalize_instruction(line) for line in function.instructions[:words])


def source_definition(source, name, *, allow_fallback=False):
    text = uncomment(source)
    if not allow_fallback and any(match[2] == name for match in INCLUDE.finditer(text)):
        return None
    # Strings can contain example code; do not treat it as a C definition.
    text = re.sub(r'"(?:\\.|[^"\\])*"|\'(?:\\.|[^\'\\])*\'',
                  lambda m: re.sub(r"[^\n]", " ", m[0]), text)
    definition = re.search(r"^[ \t]*(?:[A-Za-z_]\w*[ \t\n*]+)*" + re.escape(name) +
                           r"\s*\([^;{}]*\)\s*\{", text, re.M)
    return text.count("\n", 0, definition.start()) + 1 if definition else None


def load_references(pairs, image=None):
    references = []
    seen = set()
    for assembly_root, source_root in pairs:
        if not assembly_root.is_dir() or not source_root.is_dir():
            raise ValueError(f"reference directories must exist: {assembly_root}, {source_root}")
        source_cache = {}
        for path in sorted(assembly_root.rglob("*.s")):
            source = (source_root / path.relative_to(assembly_root).parent).with_suffix(".c")
            if not source.is_file():
                continue
            if source not in source_cache:
                source_cache[source] = source.read_text()
            text = source_cache[source]
            line = source_definition(text, path.stem)
            if line is None:
                continue
            function = parse_asm(path)
            identity = (source.resolve(), function.name)
            if identity in seen:
                continue
            if image is not None and assembly_root.resolve() == (ROOT / "asm/matchings/syslib").resolve():
                if image.read(function.address, len(function.data)) != function.data:
                    raise ValueError(f"reference differs from the verified image: {path}")
            tokens = instruction_tokens(function)
            if not tokens:
                continue
            references.append(Reference(function, source, line, tokens, Counter(tokens)))
            seen.add(identity)
    if not references:
        raise ValueError("no source-backed matching library functions found; regenerate the split first")
    return references


def complexity(function):
    metrics = function.metrics()
    return (metrics["instructions"] + 4 * metrics["branches"] + 8 * metrics["calls"] +
            2 * metrics["jumps"] + metrics["stack"] / 8 + 100 * metrics["unknown_words"])


def rank_functions(functions, references, *, minimum=0.75, min_words=8, alternatives=2):
    rows = []
    # Search slightly below the display threshold to detect close alternatives.
    ambiguity_gap = 0.03
    floor = max(0.0, minimum - ambiguity_gap)
    matchers = [(ref, ref.function.path.resolve(),
                 SequenceMatcher(None, (), ref.tokens, autojunk=False))
                for ref in references]
    for function in functions:
        tokens = instruction_tokens(function)
        if len(tokens) < min_words:
            continue
        counts = Counter(tokens)
        matches = []
        candidate_path = function.path.resolve()
        for reference, reference_path, matcher in matchers:
            if candidate_path == reference_path:
                continue
            total = len(tokens) + len(reference.tokens)
            if 2 * min(len(tokens), len(reference.tokens)) / total < floor:
                continue
            # A multiset overlap is an upper bound on ordered sequence overlap.
            overlap = sum(min(count, reference.counts[token]) for token, count in counts.items())
            if 2 * overlap / total < floor:
                continue
            matcher.set_seq1(tokens)
            blocks = matcher.get_matching_blocks()
            matched = sum(block.size for block in blocks)
            score = 2 * matched / total
            if score >= floor:
                matches.append((score, matched, reference))
        if not matches:
            continue
        matches.sort(key=lambda match: (-match[0], str(match[2].source), match[2].function.name))
        best, matched, reference = matches[0]
        if best < minimum:
            continue
        margin = best - matches[1][0] if len(matches) > 1 else None
        rows.append({
            "name": function.name, "unit": str(function.unit), "address": function.address,
            "instructions": len(tokens), "complexity": complexity(function),
            "similarity": best, "similarity_percent": round(100 * best, 2),
            "matching_instructions": matched,
            "reference": reference.function.name, "reference_source": str(reference.source),
            "reference_line": reference.line, "reference_asm": str(reference.function.path),
            "reference_instructions": len(reference.tokens),
            "ambiguous": margin is not None and margin <= ambiguity_gap + 1e-12,
            "margin_percent": round(100 * margin, 2) if margin is not None else None,
            "alternatives": [
                {"name": ref.function.name, "source": str(ref.source), "line": ref.line,
                 "similarity_percent": round(100 * score, 2)}
                for score, _, ref in matches[1:alternatives + 1]
            ],
        })
    rows.sort(key=lambda row: (-row["similarity"], row["ambiguous"],
                               row["complexity"], row["address"], row["name"]))
    for index, row in enumerate(rows, 1):
        row["rank"] = index
    return rows


def print_ranking(rows, *, as_json=False, limit=20):
    if as_json:
        import json
        print(json.dumps(rows[:limit], indent=2))
        return
    print("Structural similarity only; verify the C and linked bytes before claiming a match.")
    print("Rank Similar  Words Ambiguous  Candidate -> library reference")
    for row in rows[:limit]:
        print(f'{row["rank"]:4} {row["similarity_percent"]:6.2f}% '
              f'{row["instructions"]:5} {"yes" if row["ambiguous"] else "no":9}  '
              f'{row["name"]} -> {row["reference"]}')
        print(f'     {row["unit"]}')
        print(f'     C: {row["reference_source"]}:{row["reference_line"]}')
        for alternative in row["alternatives"]:
            print(f'     also {alternative["similarity_percent"]:.2f}% '
                  f'{alternative["name"]} ({alternative["source"]}:{alternative["line"]})')


def project_ranking(*, include_game=False, extra_references=(), minimum=0.75, min_words=8,
                    unattempted=False):
    require_fresh()
    image = PsxImage((ROOT / "disc/SLPS_018.23").read_bytes())
    pairs = [(ROOT / "asm/matchings/syslib", ROOT / "src/syslib"), *extra_references]
    references = load_references(pairs, image)
    candidates = [function for function in active_functions(sdk=True)
                  if include_game or function.unit.parts[:2] == ("src", "syslib")]
    if unattempted:
        sources = {unit: (ROOT / unit).read_text() for unit in {f.unit for f in candidates}}
        candidates = [function for function in candidates
                      if source_definition(sources[function.unit], function.name,
                                           allow_fallback=True) is None]
    for function in candidates:
        if image.read(function.address, len(function.data)) != function.data:
            raise ValueError(f"candidate differs from the verified image: {function.path}")
    return rank_functions(candidates, references, minimum=minimum, min_words=min_words)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--include-game", action="store_true", help="also rank game fallbacks; default: SDK only")
    parser.add_argument("--unattempted", action="store_true",
                        help="exclude functions with existing C, including NON_MATCHING attempts")
    parser.add_argument("--reference", nargs=2, action="append", type=Path, default=[],
                        metavar=("MATCHING_ASM_DIR", "C_SOURCE_DIR"),
                        help="additional library trees with matching relative unit paths")
    parser.add_argument("--minimum", type=float, default=75.0, metavar="PERCENT")
    parser.add_argument("--min-words", type=int, default=8, help="exclude tiny, uninformative stubs")
    parser.add_argument("--limit", type=int, default=20)
    parser.add_argument("--json", action="store_true")
    args = parser.parse_args()
    if not 0 < args.minimum <= 100 or args.min_words < 1 or args.limit < 1:
        parser.error("minimum must be in (0, 100]; min-words and limit must be positive")
    rows = project_ranking(include_game=args.include_game, extra_references=args.reference,
                           minimum=args.minimum / 100, min_words=args.min_words,
                           unattempted=args.unattempted)
    print_ranking(rows, as_json=args.json, limit=args.limit)


if __name__ == "__main__":
    try:
        main()
    except (ValueError, OSError) as error:
        raise SystemExit(str(error))
