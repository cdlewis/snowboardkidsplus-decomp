#!/usr/bin/env python3
"""PreToolUse guard shared by Codex and Claude; not a shell security boundary."""

import json
from pathlib import PurePosixPath
import shlex
import sys


def blocked_command(command):
    # shlex needs whitespace splitting to preserve paths, flags, and assignments.
    lexer = shlex.shlex(command, posix=True, punctuation_chars=";&|()\n")
    lexer.whitespace_split = True
    lexer.whitespace = " \t\r"  # retain newlines as command boundaries
    try:
        tokens = list(lexer)
    except ValueError:
        return False
    start = True
    for index, token in enumerate(tokens):
        if token and all(c in ";&|()\n" for c in token):
            start = True
            continue
        if not start:
            continue
        if "=" in token or token in {"env", "command", "exec", "sudo", "then", "do", "!"}:
            continue
        name = PurePosixPath(token).name
        rest = tokens[index + 1:]
        for end, arg in enumerate(rest):
            if arg and all(c in ";&|()\n" for c in arg):
                rest = rest[:end]
                break
        if name in {"make", "gmake", "splat", "splat64", "build.py", "clean_split_asm.py"}:
            return True
        if name.startswith("python"):
            if any(PurePosixPath(arg).name in {"build.py", "clean_split_asm.py"} for arg in rest):
                return True
            if "-m" in rest and any(arg in {"splat", "splat64", "mapfile_parser"} for arg in rest):
                return True
        if name in {"bash", "sh", "zsh"}:
            for offset, arg in enumerate(rest[:-1]):
                if arg.startswith("-") and "c" in arg and blocked_command(rest[offset + 1]):
                    return True
        start = False
    return False


def commands(event):
    if isinstance(event, dict):
        for key, value in event.items():
            if key in {"cmd", "command"} and isinstance(value, str):
                yield value
            elif isinstance(value, (dict, list)):
                yield from commands(value)
    elif isinstance(event, list):
        for value in event:
            yield from commands(value)


def main():
    event = json.load(sys.stdin)
    if any(blocked_command(command) for command in commands(event)):
        print("Blocked: use ./tools/build-and-verify.sh for builds, split regeneration, "
              "and progress reports. Use --expected to snapshot targets or "
              "--objects [--non-matching] for comparison objects. "
              "Do not bypass the wrapper with make, splat, or tools/build.py.", file=sys.stderr)
        return 2
    return 0


if __name__ == "__main__":
    sys.exit(main())
