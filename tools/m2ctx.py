#!/usr/bin/env python3
"""Generate declaration-only m2c / decomp.me context from a function's C unit."""

import argparse
import re
import subprocess
import sys
import tempfile
from pathlib import Path

import build
from decomp import INCLUDE, ROOT, uncomment

C_TOKEN = re.compile(r'''"(?:\\.|[^"\\])*"|'(?:\\.|[^'\\])*'|\w+|\S''')
C_DELIMITERS = {"(": ")", "[": "]", "{": "}"}
OLD_STYLE_PARAMETERS = re.compile(
    r"\(\s*[A-Za-z_]\w*(?:\s*,\s*[A-Za-z_]\w*)*\s*\)\s*"
    r"(?!(?:asm|__asm|__asm__)\b)[A-Za-z_]\w*"
)


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
                            + r"\s*\([^;{}]*\)\s*(?:[A-Za-z_][^;{}]*;\s*)*\{", re.M)
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


def strip_implementations(source):
    """Preserve preprocessed declarations, including struct, union, and enum bodies."""
    output = []
    start = 0
    declaration_start = 0
    delimiters = []
    previous = ""
    old_style = None
    in_initializer = False
    in_function = False
    for token in C_TOKEN.finditer(source):
        value = token[0]
        if not delimiters:
            if in_initializer and value in (",", ";"):
                start = token.start()
                in_initializer = False
            elif not in_initializer:
                # With attributes removed, a top-level brace after a
                # declarator's closing parenthesis opens a function body.
                if value == "{" and previous == ")":
                    output.extend((source[start:token.start()].rstrip(), ";"))
                    in_function = True
                elif value == "{" and previous == ";" and old_style is not None:
                    # K&R definitions put parameter declarations before the body.
                    name, signature_start, parameters_start = old_style
                    output.append(source[start:signature_start])
                    prototype = rf"\b{re.escape(name)}\s*\([^;{{}}]*\)\s*;"
                    if not re.search(prototype, "".join(output)):
                        output.extend((source[signature_start:parameters_start], "();"))
                    in_function = True
                elif value == "=":
                    output.append(source[start:token.start()].rstrip())
                    in_initializer = True
                elif value == "(" and OLD_STYLE_PARAMETERS.match(source, token.start()):
                    old_style = (previous, declaration_start, token.start())

            if value == ";":
                declaration_start = token.end()

        if value in C_DELIMITERS:
            delimiters.append(C_DELIMITERS[value])
        elif delimiters and value == delimiters[-1]:
            delimiters.pop()
            if in_function and not delimiters:
                start = token.end()
                declaration_start = start
                in_function = False
                old_style = None
        previous = value

    if delimiters or in_initializer:
        raise ValueError("unclosed declaration or implementation in preprocessed C")
    output.append(source[start:])
    return "".join(output)


def generate_context(source):
    """Keep project macros and declarations for m2c's type lookup."""
    with tempfile.TemporaryDirectory(prefix="skp-m2ctx-") as temporary:
        includes = Path(temporary)
        # Context needs only the no-op M2CTX forms, even before split generation
        # or while a clean build has removed the generated include_asm.h.
        (includes / "include_asm.h").write_text(
            "#pragma once\n#define INCLUDE_ASM(FOLDER, NAME)\n"
            "#define INCLUDE_RODATA(FOLDER, NAME)\n")
        command = [build.CPP, *build.CPP_FLAGS, "-I" + str(includes),
                   "-DM2CTX", "-DNON_MATCHING", "-D__attribute__(x)=", "-P"]
        stock = subprocess.check_output(command + ["-dM", "-x", "c", "-"],
                                        cwd=ROOT, input="", text=True)
        macros = subprocess.check_output(command + ["-dM", str(source)], cwd=ROOT, text=True)
        preprocessed = subprocess.check_output(command + [str(source)], cwd=ROOT, text=True)
        predefined = set(stock.splitlines())
        # -dM preserves unexpanded macro bodies, including storage annotations.
        # Those annotation macros are unnecessary in already-expanded C.
        project_macros = [line for line in macros.splitlines()
                          if line not in predefined and "__attribute__" not in line]
        return "\n".join(project_macros) + "\n\n" + strip_implementations(preprocessed)


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
