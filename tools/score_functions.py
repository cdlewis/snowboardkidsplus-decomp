#!/usr/bin/env python3
"""Rank live unmatched game functions."""

import argparse
import json

from decomp import active_functions


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--sdk", action="store_true", help="also include PsyQ functions")
    parser.add_argument("--limit", type=int, default=20)
    parser.add_argument("--json", action="store_true")
    args = parser.parse_args()
    rows = []
    for function in active_functions(sdk=args.sdk):
        metrics = function.metrics()
        # Rank by instruction count, control flow, calls, stack, and unknown words.
        score = (metrics["instructions"] + 4 * metrics["branches"] +
                 8 * metrics["calls"] + 2 * metrics["jumps"] +
                 metrics["stack"] / 8 + 100 * metrics["unknown_words"])
        rows.append({"name": function.name, "unit": str(function.unit),
                     "address": function.address, "heuristic": score, **metrics})
    rows.sort(key=lambda r: (r["heuristic"], r["address"]))
    if args.json:
        print(json.dumps(rows[:args.limit], indent=2))
    else:
        print("Heuristic  Insn Branch Call Stack  Function / source unit")
        for r in rows[:args.limit]:
            print(f'{r["heuristic"]:9.1f} {r["instructions"]:5} {r["branches"]:6} '
                  f'{r["calls"]:4} {r["stack"]:5}  {r["name"]}  {r["unit"]}')


if __name__ == "__main__":
    try:
        main()
    except ValueError as error:
        raise SystemExit(str(error))
