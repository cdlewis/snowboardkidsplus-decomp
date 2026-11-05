#!/usr/bin/env python3
"""Run native CCPSX GCC, preserving explicit sections on external objects.

GCC 2.7.2-cdk emits .extern NAME,SIZE even when an external declaration has
section(".data"). ASPSX/maspsx interpret that directive as small-data eligibility.
GCC 2.8.1 omits it for explicitly placed objects. Retain that declaration's
section information here, without changing any compiler-generated instructions.
"""

from pathlib import Path
import re
import subprocess
import sys


ROOT = Path(__file__).resolve().parent.parent
COMPILER = ROOT / "tools/gcc/gcc-2.7.2-cdk/cc1"


def absolute_externs(source):
    declarations = re.finditer(
        r'\bextern\b([^;{}]*?)__attribute__\s*\(\(\s*section\s*'
        r'\(\s*"([^"\n]+)"\s*\)\s*\)\)\s*;', source)
    result = {}
    for declaration in declarations:
        section = declaration[2]
        if section.split(".")[1:2] in (["sdata"], ["sbss"]):
            continue
        name = re.search(r'\b(\w+)\s*(?:\[[^\]]*\]\s*)*$', declaration[1])
        if name:
            result[name[1]] = section
    return result


def main():
    args = sys.argv[1:]
    if not COMPILER.is_file():
        sys.exit("Missing gcc-2.7.2-cdk/cc1; run tools/fetch-gcc.py")
    result = subprocess.run([str(COMPILER), *args])
    if result.returncode:
        return result.returncode
    if "-o" not in args:
        return 0
    output_index = args.index("-o") + 1
    sources = [Path(arg) for i, arg in enumerate(args)
               if not arg.startswith("-") and i != output_index and Path(arg).is_file()]
    if len(sources) != 1:
        sys.exit("CCPSX adapter requires one preprocessed C input and -o OUTPUT")
    absolute = absolute_externs(sources[0].read_text())
    output = Path(args[output_index])
    assembly = output.read_text()

    def preserve_section(match):
        name = match[1]
        if name in absolute:
            return f"# {name} is declared in {absolute[name]}; no small-data .extern"
        return match[0]

    output.write_text(re.sub(r'^\s*\.extern\s+(\w+),\s*\d+\s*$',
                             preserve_section, assembly, flags=re.M))
    return 0


if __name__ == "__main__":
    sys.exit(main())
