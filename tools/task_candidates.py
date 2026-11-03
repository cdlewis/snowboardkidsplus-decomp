#!/usr/bin/env python3
"""Emit task-runner JSON candidates using the project's PSX ranking tools."""

import argparse
import json

from decomp import active_functions, require_fresh
from rank_library_functions import complexity, project_ranking


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("task", choices=("decompile-similar", "decompile-unmatched",
                                         "decompile-library"))
    parser.add_argument("--limit", type=int, help="limit candidates; default: all")
    args = parser.parse_args()
    if args.limit is not None and args.limit < 1:
        parser.error("limit must be positive")

    if args.task == "decompile-similar":
        rows = project_ranking(include_game=True)
        candidates = [
            [row["name"],
             f'{row["reference"]} ({row["reference_source"]}:{row["reference_line"]})',
             *[f'{ref["name"]} ({ref["source"]}:{ref["line"]})'
               for ref in row["alternatives"]]]
            for row in rows if not row["ambiguous"]
        ]
    elif args.task == "decompile-library":
        rows = project_ranking()
        priority = {row["name"]: index for index, row in enumerate(
                    row for row in rows if not row["ambiguous"])}
        functions = [f for f in active_functions(sdk=True)
                     if f.unit.parts[:2] == ("src", "syslib")]
        functions.sort(key=lambda f: (priority.get(f.name, len(priority)),
                                      complexity(f), f.address, f.name))
        candidates = [function.name for function in functions]
    else:
        require_fresh()
        functions = sorted(active_functions(sdk=True),
                           key=lambda f: (complexity(f), f.address, f.name))
        candidates = [function.name for function in functions]

    print(json.dumps(candidates[:args.limit], indent=2))


if __name__ == "__main__":
    try:
        main()
    except (ValueError, OSError) as error:
        raise SystemExit(str(error))
