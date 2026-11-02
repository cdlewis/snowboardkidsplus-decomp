"""Clean, regenerate, build, verify, and report through one entry point."""

import argparse
import os
from pathlib import Path
import subprocess
import sys

from decomp import INCLUDE, uncomment
from validate_commit import verify_image

ROOT = Path(__file__).resolve().parent.parent


def check_includes(root):
    for source in sorted((root / "src").rglob("*.c")):
        for match in INCLUDE.finditer(uncomment(source.read_text())):
            path = root / match[1] / (match[2] + ".s")
            if not path.is_file():
                raise ValueError(f"{source.relative_to(root)}: missing INCLUDE_ASM file {path}")
    print("[verify] Every INCLUDE_ASM has a generated file.", flush=True)


def build_and_verify(root, *, expected=False, objects=False, non_matching=False):
    environment = dict(os.environ)
    environment.pop("NON_MATCHING", None)

    def make(target, **extra):
        print(f"[verify] make {target}", flush=True)
        subprocess.run(["make", target], cwd=root, env=environment | extra, check=True)

    if not (root / "disc/SLPS_018.23").exists():
        make("extract")
    make("clean")
    make("split")
    check_includes(root)
    make("all")
    verify_image(root)
    make("report")
    if expected:
        # The full build above was independently verified before snapshotting.
        subprocess.run([sys.executable, "-c", "from build import update_expected; update_expected()"],
                       cwd=root / "tools", env=environment, check=True)
    if objects:
        make("skipasm", NON_MATCHING="1" if non_matching else "0")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--expected", action="store_true", help="snapshot verified target objects")
    parser.add_argument("--objects", action="store_true", help="prepare SKIP_ASM comparison objects after verification")
    parser.add_argument("--non-matching", action="store_true", help="also compile partial C (requires --objects)")
    args = parser.parse_args()
    if args.non_matching and not args.objects:
        parser.error("--non-matching requires --objects")
    try:
        build_and_verify(ROOT, expected=args.expected, objects=args.objects, non_matching=args.non_matching)
    except (OSError, ValueError, subprocess.CalledProcessError) as error:
        print(f"BUILD HAS FAILED. {error}", file=sys.stderr)
        return 1
    print("BUILD SUCCEEDED. Full executable matches the protected SHA1; progress is fresh.", flush=True)
    if args.objects:
        print("Comparison objects prepared; they are not proof of additional matches.", flush=True)
    return 0


if __name__ == "__main__":
    sys.exit(main())
