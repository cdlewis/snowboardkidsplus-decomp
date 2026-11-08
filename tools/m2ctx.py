#!/usr/bin/env python3
"""Generate m2c / decomp.me context from a function's owning C unit."""

import argparse
import re
import subprocess
import sys
from pathlib import Path

import build
from decomp import INCLUDE, ROOT, uncomment


def find_unit(function, requested_unit=None):
    if not re.fullmatch(r"[A-Za-z_]\w*", function):
        raise ValueError("provide a C function name")
    if requested_unit is not None:
        unit = requested_unit
        if (unit.is_absolute() or ".." in unit.parts or unit.suffix != ".c"
                or not (ROOT / unit).is_file()):
            raise ValueError("--unit must identify an existing repository-relative C source")
        sources = [ROOT / unit]
    else:
        sources = sorted((ROOT / "src").rglob("*.c"))
    definition = re.compile(r"^[^\n;{}#]*\b" + re.escape(function)
                            + r"\s*\([^;{}]*\)\s*\{", re.M)
    matches = []
    for source in sources:
        text = uncomment(source.read_text())
        if any(m[2] == function for m in INCLUDE.finditer(text)) or definition.search(text):
            matches.append(source.relative_to(ROOT))
    if not matches:
        raise ValueError(f"no C owner found for {function}")
    if len(matches) != 1:
        raise ValueError(f"ambiguous owner for {function}: "
                         + ", ".join(map(str, matches)) + "; select one with --unit")
    return matches[0]


def generate_context(source):
    """Preprocess PSX declarations without compiler-specific attributes."""
    return subprocess.check_output(
        [build.CPP, *build.CPP_FLAGS, "-DM2CTX", "-DNON_MATCHING",
         "-D__attribute__(x)=", "-P", str(source)],
        cwd=ROOT, text=True)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("function", help="matched or assembly-backed function name")
    parser.add_argument("-o", "--output", type=Path, default=Path("ctx.c"),
                        help="output path (default: ctx.c in the current directory)")
    parser.add_argument("--unit", type=Path, help="owning src/path.c if the name is ambiguous")
    args = parser.parse_args()
    unit = find_unit(args.function, args.unit)
    context = generate_context(ROOT / unit)
    args.output.write_text(context)
    print(f"Context for {args.function} from {unit}: {args.output.resolve()}")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (ValueError, OSError, subprocess.CalledProcessError) as error:
        print(f"Context error: {error}", file=sys.stderr)
        raise SystemExit(2)
