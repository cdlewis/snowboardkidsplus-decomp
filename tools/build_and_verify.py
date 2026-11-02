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


def build_and_verify(root, *, expected=False, objects=False, non_matching=False,
                     incremental=False, jobs=1):
    environment = dict(os.environ)
    environment.pop("NON_MATCHING", None)

    def make(target, **extra):
        print(f"[verify] make {target}", flush=True)
        subprocess.run(["make", f"-j{jobs}", target], cwd=root, env=environment | extra, check=True)

    if not (root / "disc/SLPS_018.23").exists():
        make("extract")
    if not incremental:
        make("clean")
    make("split")
    check_includes(root)
    make("all")
    verify_image(root)
    make("report")
    if expected:
        # The full build above was independently verified before snapshotting.
        make("expected")
    if objects:
        make("objects", NON_MATCHING="1" if non_matching else "0")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--expected", action="store_true", help="snapshot verified target objects")
    parser.add_argument("--objects", action="store_true", help="prepare comparison objects after verification")
    parser.add_argument("--non-matching", action="store_true", help="also compile partial C (requires --objects)")
    parser.add_argument("--incremental", action="store_true", help="verify without cleaning unchanged outputs")
    parser.add_argument("--jobs", type=int, default=1, help="parallel Make jobs (default: 1)")
    args = parser.parse_args()
    if args.jobs < 1:
        parser.error("--jobs must be positive")
    if args.non_matching and not args.objects:
        parser.error("--non-matching requires --objects")
    try:
        build_and_verify(ROOT, expected=args.expected, objects=args.objects, non_matching=args.non_matching,
                         incremental=args.incremental, jobs=args.jobs)
    except (OSError, ValueError, subprocess.CalledProcessError) as error:
        print(f"BUILD HAS FAILED. {error}", file=sys.stderr)
        return 1
    print("BUILD SUCCEEDED. Full executable matches the protected SHA1; progress is fresh.", flush=True)
    return 0


if __name__ == "__main__":
    sys.exit(main())
