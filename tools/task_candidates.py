#!/usr/bin/env python3
"""Emit task-runner JSON candidates using the project's PSX ranking tools."""

import argparse
import json
import re
import struct

from decomp import WORD, active_functions, require_fresh
from build import get_settings
from rank_library_functions import complexity, project_ranking


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
        flags = settings["CC1_FLAGS"] + settings["UNIT_CC_FLAGS"] + settings["FUNCTION_FLAGS"]
        # Verified C also has this shape when scheduling is disabled. Restrict
        # the heuristic to the game's normal optimizing/reordering pipeline.
        if (settings["REORDER"] == "1" and "-O2" in flags
                and "-fno-delayed-branch" not in flags):
            return False
    return True


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("task", choices=("decompile-similar", "decompile-unmatched",
                                         "decompile-library"))
    parser.add_argument("--limit", type=int, help="limit candidates; default: all")
    args = parser.parse_args()
    if args.limit is not None and args.limit < 1:
        parser.error("limit must be positive")

    require_fresh()
    functions = [f for f in active_functions(sdk=True)
                 if is_c_candidate(f)]
    eligible = {f.name for f in functions}

    if args.task == "decompile-similar":
        rows = project_ranking(include_game=True)
        candidates = [
            [row["name"],
             f'{row["reference"]} ({row["reference_source"]}:{row["reference_line"]})',
             *[f'{ref["name"]} ({ref["source"]}:{ref["line"]})'
               for ref in row["alternatives"]]]
            for row in rows if not row["ambiguous"] and row["name"] in eligible
        ]
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
