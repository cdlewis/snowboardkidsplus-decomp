#!/usr/bin/env python3
"""Merge selected compiler-generated functions without changing their instructions."""

import argparse
import re
from pathlib import Path


def replace_compiler_function(normal: str, alternate: str, name: str) -> str:
    """Select one complete .ent/.end region from an alternate compilation.

    The original data, declarations, and other functions remain in the normal
    compilation. Rename colliding compiler-local labels in the selected region;
    this changes symbol names only, never instruction content or scheduling.
    """
    symbol = re.escape(name)
    pattern = re.compile(
        rf"^[ \t]*\.ent[ \t]+{symbol}[ \t]*\n"
        rf".*?^[ \t]*\.end[ \t]+{symbol}[ \t]*(?:\n|$)",
        re.MULTILINE | re.DOTALL,
    )
    original = list(pattern.finditer(normal))
    replacement = list(pattern.finditer(alternate))
    if len(original) != 1 or len(replacement) != 1:
        raise ValueError(f"expected one compiler function region for {name}")
    start, end = original[0].span()
    outside = normal[:start] + normal[end:]
    region = replacement[0].group()
    labels = re.findall(r"^\s*(\$L\d+):", region, re.MULTILINE)
    for label in labels:
        if re.search(rf"^{re.escape(label)}:", outside, re.MULTILINE):
            alternate_outside = alternate[:replacement[0].start()] + alternate[replacement[0].end():]
            if re.search(rf"{re.escape(label)}(?![\w$])", alternate_outside):
                raise ValueError(f"cannot isolate externally referenced label {label} in {name}")
            region = re.sub(rf"{re.escape(label)}(?![\w$])", f"{label}_{name}", region)
    return normal[:start] + region + normal[end:]



def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("base", type=Path)
    parser.add_argument("output", type=Path)
    parser.add_argument("functions", nargs="*", help="function=alternate assembly path")
    args = parser.parse_args()
    assembly = args.base.read_text()
    for selection in args.functions:
        name, filename = selection.split("=", 1)
        assembly = replace_compiler_function(assembly, Path(filename).read_text(), name)
    args.output.write_text(assembly)


if __name__ == "__main__":
    main()
