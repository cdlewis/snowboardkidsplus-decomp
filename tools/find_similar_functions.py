#!/usr/bin/env python3
"""Find structural assembly references, not byte matches or proven ports.

Adapted from snowboardkids2-decomp's register normalization, instruction
comparison, and control-flow reference search. No embeddings/cache dependency.
"""

import argparse
import re
from difflib import SequenceMatcher
from pathlib import Path

from decomp import ROOT, active_functions, parse_asm

REGISTERS = ("zero at v0 v1 a0 a1 a2 a3 t0 t1 t2 t3 t4 t5 t6 t7 "
             "s0 s1 s2 s3 s4 s5 s6 s7 t8 t9 k0 k1 gp sp fp ra").split()


def normalize_instruction(line):
    def register(match):
        name = match[0][1:]
        if name.isdigit() and int(name) < 32:
            name = REGISTERS[int(name)]
        return "$" + re.sub(r'^([vast])\d$', r'\1N', name)
    line = re.sub(r'\$(?:\d+|[a-z][a-z0-9]*)', register, line.split("#", 1)[0])
    line = re.sub(r'%(hi|lo|gp_rel)\([^)]*\)', r'%\1(SYM)', line)
    line = re.sub(r'\.L[\da-fA-F_]+', 'LABEL', line)
    line = re.sub(r'\bjal\s+\S+', 'jal FUNC', line)
    return " ".join(line.split())


def similarity(left, right):
    a = [normalize_instruction(i) for i in left.instructions]
    b = [normalize_instruction(i) for i in right.instructions]
    return SequenceMatcher(None, a, b, autojunk=False).ratio()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("function")
    parser.add_argument("--top", type=int, default=10)
    parser.add_argument("--sdk", action="store_true")
    parser.add_argument("--asm-dir", action="append", type=Path, default=[],
                        help="explicit additional reference directory, e.g. a sibling's asm tree")
    args = parser.parse_args()
    functions = list(active_functions(sdk=args.sdk))
    query = next((f for f in functions if f.name == args.function), None)
    if query is None:
        path = Path(args.function)
        if not path.is_file():
            raise ValueError("function is not an active include; supply an assembly path explicitly")
        query = parse_asm(path)
    for directory in args.asm_dir:
        if not directory.is_dir():
            raise ValueError(f"reference directory does not exist: {directory}")
        for path in directory.rglob("*.s"):
            try:
                functions.append(parse_asm(path))
            except ValueError:
                continue
    results = []
    seen = {query.path.resolve()}
    for function in functions:
        path = function.path.resolve()
        if path in seen:
            continue
        seen.add(path)
        ratio = len(function.instructions) / max(1, len(query.instructions))
        if 0.4 <= ratio <= 2.5:
            results.append((similarity(query, function), function))
    print("Structural similarity only; inspect the C and verify linked bytes.")
    for score, function in sorted(results, key=lambda item: item[0], reverse=True)[:args.top]:
        print(f"{score:.3f}  {function.name:35} {function.path}")


if __name__ == "__main__":
    try:
        main()
    except ValueError as error:
        raise SystemExit(str(error))
