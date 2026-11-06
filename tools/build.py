#!/usr/bin/env python3
"""Make settings and scratch-build adapter for the matching tools.

The Makefile owns compilation, assembly, linking and verification. This module
retains the protected checksum and lets isolated attempts use those same rules.
"""

import os
import shlex
import subprocess
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
TARGET_SHA1 = "3f1a7abc267ea29bafed1653121613ea467c1ac8"


def get_settings(unit=None, function=None, *, comparison=False):
    command = ["make", "--no-print-directory", "-s", "print-settings",
               f"UNIT={unit or ''}", f"ONLY_FUNCTION={function or ''}",
               f"COMPARISON={int(comparison)}",
               f"BUILD_DIR={'build/compare' if comparison else 'build'}",
               f"NON_MATCHING={os.environ.get('NON_MATCHING', '0') if comparison else '0'}"]
    output = subprocess.check_output(command, cwd=ROOT, text=True)
    values = dict(line.split("=", 1) for line in output.splitlines())
    for name in ("CPP_FLAGS", "AS_FLAGS", "ORACLE_AS_FLAGS", "CC1_FLAGS",
                 "UNIT_CC_FLAGS"):
        values[name] = shlex.split(values[name])
    return values


TOOLS = get_settings()
CPP = TOOLS["CPP"]
CPP_FLAGS = TOOLS["CPP_FLAGS"]
AS = TOOLS["AS"]
LD = TOOLS["LD"]
CROSS = TOOLS["CROSS"]
ASPSX_VERSION = TOOLS["ASPSX_VERSION"]


def compile_c(src, comparison, *, output_root, unit_path,
              only_function=None, extra_flags=()):
    if not comparison:
        raise ValueError("use the Makefile for executable builds")
    output_root = Path(output_root).resolve()
    if output_root.is_relative_to(ROOT) and not output_root.is_relative_to(ROOT / "nonmatchings"):
        raise ValueError("scratch outputs must stay outside the executable build")
    command = ["make", "--no-print-directory", "scratch", "COMPARISON=1",
               f"NON_MATCHING={os.environ.get('NON_MATCHING', '0')}",
               f"BUILD_DIR={output_root}", f"UNIT={unit_path}",
               f"SOURCE={Path(src).resolve()}", f"ONLY_FUNCTION={only_function or ''}",
               f"EXTRA_CC_FLAGS={shlex.join(extra_flags)}"]
    subprocess.run(command, cwd=ROOT, check=True)
    return output_root / Path(unit_path).with_suffix(".o")


if __name__ == "__main__":
    raise SystemExit("Build with Make; agents use ./tools/build-and-verify.sh.")
