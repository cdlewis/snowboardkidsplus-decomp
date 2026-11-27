#!/usr/bin/env python3
"""Emit task-runner JSON candidates using the project's PSX ranking tools."""

import argparse
import json
import re
import struct
from hashlib import sha256
from pathlib import Path
from tempfile import NamedTemporaryFile

from decomp import ROOT, WORD, Elf32, active_functions, require_fresh
from build import get_settings
from progress_attempts import MARKER, read_attempts
from symbol_cleanup_candidates import cleanup_groups
from rank_library_functions import complexity, print_ranking, project_ranking, source_definition


SIMILAR_CACHE = ROOT / "nonmatchings/.similar-references.json"
# Store the original addresses so Nigel's candidate key survives a match or rename.
DECOMPILE_DUOS = (
    ("src/graphics/sprite_draw.c", (0x8006EF30, 0x8006F248)),
)
# The renderer wrapper saves registers in static storage. Its two helpers
# receive arguments in temporary registers through direct calls.
UNMATCHED_RENDERER_ENTRY_POINTS = {0x800585BC, 0x80058664, 0x80058844}


def duo_statuses():
    active = {function.address: function for function in active_functions(sdk=True)}
    attempts = read_attempts(ROOT)
    linked = Elf32(ROOT / "build/slps_018.23.elf").symbols
    duos = []
    for unit_name, addresses in DECOMPILE_DUOS:
        duo = []
        source = (ROOT / unit_name).read_text()
        for address in addresses:
            target = f"func_{address:08X}"
            if fallback := active.get(address):
                attempt = attempts[fallback.unit, fallback.name]
                lines = (ROOT / fallback.unit).read_text().splitlines()
                marker = MARKER.fullmatch(lines[attempt.line - 1].strip())
                duo.append({"target": target, "status": "fallback", "symbol": fallback.name,
                            "source": f"{fallback.unit}:{attempt.line}", "attempt": marker[1]})
                continue
            for symbol in linked.values():
                if symbol.kind == 2 and symbol.address == address:
                    if line := source_definition(source, symbol.name):
                        duo.append({"target": target, "status": "exact", "symbol": symbol.name,
                                    "source": f"{unit_name}:{line}"})
                        break
            else:
                duo.append({"target": target, "status": "unresolved", "source": unit_name})
        duos.append(duo)
    return duos


def duos_with_remaining_work(duos):
    return [[member["target"] for member in duo]
            for duo in duos if any(member["status"] != "exact" for member in duo)]


def similar_inputs():
    # Source locations survive a clean regeneration of the verified build.
    # Include sources and split inputs, rather than generated ASM timestamps.
    paths = sorted((ROOT / "src").rglob("*.c"))
    paths.extend(ROOT / name for name in (
        "snowboardkidsplus.yaml", "symbol_addrs.txt", "reloc_addrs.txt",
        "tools/task_candidates.py", "tools/rank_library_functions.py",
        "tools/find_similar_functions.py", "tools/decomp.py",
    ))
    digest = sha256()
    for path in paths:
        stat = path.stat()
        stamp = f"{stat.st_size}:{stat.st_mtime_ns}:{stat.st_ctime_ns}"
        digest.update(f"{path.relative_to(ROOT)}\0{stamp}\n".encode())
    return digest.hexdigest()


def similar_ranking(function_name=None):
    return project_ranking(include_game=True, include_game_references=True,
                           minimum=0.40, function_name=function_name)


def cache_similar_rows(inputs, rows):
    SIMILAR_CACHE.parent.mkdir(parents=True, exist_ok=True)
    with NamedTemporaryFile(mode="w", dir=SIMILAR_CACHE.parent, delete=False) as output:
        temporary = Path(output.name)
        try:
            json.dump({"inputs": inputs, "rows": {row["name"]: row for row in rows}}, output)
            output.close()
            temporary.replace(SIMILAR_CACHE)
        finally:
            temporary.unlink(missing_ok=True)


def similar_reference_row(function_name):
    require_fresh()
    try:
        cache = json.loads(SIMILAR_CACHE.read_text())
        if cache["inputs"] == similar_inputs():
            return cache["rows"].get(function_name)
    except (OSError, ValueError, KeyError, AttributeError, TypeError):
        pass
    rows = similar_ranking(function_name)
    return rows[0] if rows else None


def body_words(function):
    """Decode the function body, excluding data/alignment after endlabel."""
    body = re.split(r"^end(?:d)?label\s+" + re.escape(function.name) + r"\b",
                    function.path.read_text(), maxsplit=1, flags=re.M)[0]
    count = sum(bool(WORD.search(line)) for line in body.splitlines())
    return struct.unpack("<" + "I" * count, function.data[:count * 4])


def is_store_only_stub(words):
    """Recognize unscheduled entry stubs, not arbitrary leaf functions/nops.

    These immediately store incoming arguments/zero through gp or sp, then
    return without any other work. Normal game C scheduling puts the final
    store in the return delay slot. This is a candidate-selection
    heuristic, not proof of handwritten assembly or an o32 ABI violation.
    """
    return (len(words) >= 3 and words[-2:] == (0x03e00008, 0)
            and all(word >> 26 in {40, 41, 43}
                    and (word >> 21) & 31 in {28, 29}
                    and (word >> 16) & 31 in {0, 4, 5, 6, 7}
                    for word in words[:-2]))


def follows_c_calling_convention(function, words=None):
    """Reject clear o32 ABI violations, without claiming to identify assembly.

    Temporary/result registers cannot supply inputs at entry, and s0-s7/fp
    must be preserved. Track writes conservatively across the whole body:
    this can miss violations on individual paths, but avoids rejecting C just
    because a branch has a different register lifetime. Unknown instructions
    are left alone rather than guessed at.
    """
    if words is None:
        words = body_words(function)
    defined = {0, 4, 5, 6, 7, 16, 17, 18, 19, 20, 21, 22, 23, 28, 29, 30, 31}
    inputs = {2, 3, *range(8, 16), 24, 25, 26, 27}
    preserved = {*range(16, 24), 30}
    written = set()
    saves, restores = set(), set()
    for word in words:
        op, rs, rt, rd = word >> 26, (word >> 21) & 31, (word >> 16) & 31, (word >> 11) & 31
        reads, writes = set(), set()
        if op == 0:
            fn = word & 63
            if fn in {0, 2, 3}:  # fixed shifts
                reads, writes = {rt}, {rd}
            elif fn in {4, 6, 7, 32, 33, 34, 35, 36, 37, 38, 39, 42, 43}:
                reads, writes = {rs, rt}, {rd}
            elif fn in {8, 9}:
                reads = {rs}
                if fn == 9:
                    writes = {rd, 2, 3}
            elif fn in {16, 18}:  # mfhi/mflo
                writes = {rd}
            elif fn in {17, 19}:
                reads = {rs}
            elif fn in {24, 25, 26, 27}:
                reads = {rs, rt}
        elif op == 3:  # jal; C results become available after the call
            writes = {31, 2, 3}
        elif op in {4, 5}:
            reads = {rs, rt}
        elif op in {1, 6, 7}:
            reads = {rs}
        elif op in range(8, 15):
            reads, writes = {rs}, {rt}
        elif op == 15:
            writes = {rt}
        elif op in {32, 33, 34, 35, 36, 37, 38}:
            # Treat the first half of a paired unaligned load as a definition.
            reads, writes = {rs}, {rt}
            if op == 35 and rs == 29:
                restores.add((rt, word & 0xffff))
        elif op in {40, 41, 42, 43, 46}:
            reads = {rs, rt}
            if op == 43 and rs == 29:
                saves.add((rt, word & 0xffff))
        elif op in {16, 18}:  # mfc/cfc and mtc/ctc (including GTE)
            if rs in {0, 2}:
                writes = {rt}
            elif rs in {4, 6}:
                reads = {rt}
        elif op in {50, 58}:  # lwc2/swc2
            reads = {rs}
        if (reads & inputs) - defined:
            return False
        defined.update(writes)
        written.update(writes & preserved)
    return all(any(reg == saved_reg for saved_reg, _ in saves & restores)
               for reg in written)


def is_c_candidate(function):
    words = body_words(function)
    if not follows_c_calling_convention(function, words):
        return False
    if is_store_only_stub(words):
        settings = get_settings(function.unit, function.name, comparison=True)
        flags = settings["CC1_FLAGS"] + settings["UNIT_CC_FLAGS"]
        # Verified C also has this shape when scheduling is disabled. Restrict
        # the heuristic to the game's normal optimizing/reordering pipeline.
        if (settings["REORDER"] == "1" and "-O2" in flags
                and "-fno-delayed-branch" not in flags):
            return False
    return True


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("task", choices=("decompile-similar", "decompile-unmatched",
                                         "decompile-library", "decompile-duos", "cleanup-symbols"))
    parser.add_argument("--limit", type=int, help="limit candidates; default: all")
    parser.add_argument("--references", metavar="FUNCTION",
                        help="show the matched C references for a decompile-similar candidate")
    parser.add_argument("--duo-status", action="store_true",
                        help="show the current source and match status for each decompile-duos member")
    parser.add_argument("--symbol-details", action="store_true",
                        help="show names, reasons, and source context for cleanup-symbols")
    args = parser.parse_args()
    if args.limit is not None and args.limit < 1:
        parser.error("limit must be positive")
    if args.references and args.task != "decompile-similar":
        parser.error("--references requires decompile-similar")
    if args.duo_status and args.task != "decompile-duos":
        parser.error("--duo-status requires decompile-duos")

    if args.references:
        row = similar_reference_row(args.references)
        if row is not None:
            print_ranking([dict(row, rank=1)], limit=1)
        else:
            print(f"No ranked references for {args.references}.")
        return

    if args.symbol_details and args.task != "cleanup-symbols":
        parser.error("--symbol-details requires cleanup-symbols")
    if args.task == "cleanup-symbols":
        groups = cleanup_groups()[:args.limit]
        results = groups if args.symbol_details else [
            [member["address"] for member in group] for group in groups]
        print(json.dumps(results, indent=2))
        return

    require_fresh()
    if args.task == "decompile-duos":
        duos = duo_statuses()
        results = duos if args.duo_status else duos_with_remaining_work(duos)
        print(json.dumps(results[:args.limit], indent=2))
        return

    functions = [f for f in active_functions(sdk=True)
                 if ((args.task == "decompile-unmatched"
                      and f.address in UNMATCHED_RENDERER_ENTRY_POINTS)
                     or is_c_candidate(f))]
    eligible = {f.name for f in functions}

    if args.task == "decompile-similar":
        inputs = similar_inputs()
        rows = similar_ranking()
        # Keep clear, high-scoring references first, followed by the wider queue
        # in similarity order. Nigel tracks the names across reference changes.
        rows.sort(key=lambda row: row["ambiguous"] or row["similarity"] < 0.75)
        candidates = [row["name"] for row in rows if row["name"] in eligible]
        try:
            cache_similar_rows(inputs, rows)
        except OSError:
            pass  # Candidate selection also works with a read-only scratch directory.
    elif args.task == "decompile-library":
        rows = project_ranking()
        priority = {row["name"]: index for index, row in enumerate(
                    row for row in rows if not row["ambiguous"])}
        functions = [f for f in functions
                     if f.unit.parts[:2] == ("src", "syslib")]
        functions.sort(key=lambda f: (priority.get(f.name, len(priority)),
                                      complexity(f), f.address, f.name))
        candidates = [function.name for function in functions]
    else:
        functions = sorted(functions,
                           key=lambda f: (complexity(f), f.address, f.name))
        candidates = [function.name for function in functions]

    print(json.dumps(candidates[:args.limit], indent=2))


if __name__ == "__main__":
    try:
        main()
    except (ValueError, OSError) as error:
        raise SystemExit(str(error))
