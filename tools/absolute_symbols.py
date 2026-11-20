"""Emit linker definitions for absolute symbols that Splat does not export."""

import argparse
import re
from pathlib import Path


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("symbols", type=Path)
    parser.add_argument("output", type=Path)
    args = parser.parse_args()

    lines = ["/* Generated from symbol_addrs.txt. These symbols allocate no storage. */"]
    for number, line in enumerate(args.symbols.read_text().splitlines(), 1):
        declaration, _, attributes = line.partition("//")
        if not declaration.strip() or "absolute:true" not in attributes.split():
            continue
        match = re.fullmatch(r"\s*([\w.$]+)\s*=\s*(0x[\da-fA-F]+);\s*", declaration)
        if match is None:
            parser.error(f"{args.symbols}:{number}: invalid absolute symbol declaration")
        name, address = match.groups()
        lines.append(f"{name} = {address};")
    args.output.write_text("\n".join(lines) + "\n")


if __name__ == "__main__":
    main()
