#!/usr/bin/env python3
"""Prepare and run bounded decomp-permuter searches with the real PSX pipeline."""

import argparse
import json
import math
import os
from pathlib import Path
import shlex
import shutil
import signal
import subprocess
import sys
import tempfile

import build
from decomp_env import bootstrap, check_image, resolve_function

ROOT = build.ROOT


def compile_candidate(workspace, source, output):
    metadata = json.loads((workspace / "workspace.json").read_text())
    # Workers must never share intermediate preprocessor/assembler outputs.
    with tempfile.TemporaryDirectory(prefix="psx-permuter-") as scratch:
        obj = build.compile_c(source, True, output_root=Path(scratch),
                              unit_path=Path(metadata["unit"]),
                              only_function=metadata["function"])
        shutil.copyfile(obj, output)


def prepare(args, permuter):
    check_image()
    oracle = resolve_function(args.function, args.unit)
    source = args.source_file.resolve()
    if not source.is_file():
        raise ValueError(f"source does not exist: {source}")
    workspace = bootstrap(oracle, None)
    experiment = workspace / "permuter"
    experiment.mkdir()
    shutil.copyfile(source, experiment / "original.c")
    settings = build.get_settings(oracle.unit, oracle.name, comparison=True)
    # Flatten real project headers once. PERM_* invocations survive cpp and are
    # expanded by decomp-permuter before compile.sh sees each candidate.
    subprocess.run([settings["CPP"], *settings["CPP_FLAGS"], "-DNON_MATCHING",
                    "-DPERMUTER", "-I" + str(source.parent),
                    "-I" + str((ROOT / oracle.unit).parent), "-P", str(source),
                    "-o", str(experiment / "base.c")], cwd=ROOT, check=True)
    subprocess.run([sys.executable, str(permuter / "strip_other_fns.py"),
                    str(experiment / "base.c"), oracle.name], check=True)
    shutil.copyfile(workspace / "target.o", experiment / "target.o")
    shutil.copyfile(workspace / "workspace.json", experiment / "workspace.json")
    command = shlex.join([sys.executable, str(ROOT / "tools/permuter.py"),
                          "--compile", str(experiment)])
    wrapper = experiment / "compile.sh"
    wrapper.write_text('#!/bin/sh\nset -eu\nexec ' + command + ' "$@"\n')
    wrapper.chmod(0o755)
    (experiment / "settings.toml").write_text(
        'compiler_type = "gcc"\nfunc_name = ' + json.dumps(oracle.name) +
        '\nobjdump_command = ' + json.dumps(shlex.join(
            [settings["CROSS"] + "objdump", "-drz", "-m", "mips:3000"])) + '\n')
    # Smoke-test the exact wrapper used by workers before starting a search.
    subprocess.run([str(wrapper), str(experiment / "base.c"), "-o",
                    str(experiment / "base.o")], cwd=ROOT, check=True)
    print(f"Prepared experiment: {experiment}", flush=True)
    print("Scores are search hints. Check winners with tools/match_function.py; "
          "integration still requires the full-image gate.", flush=True)
    return experiment


def run_search(args, permuter, experiment):
    command = [sys.executable, str(permuter / "permuter.py"), "-j", str(args.jobs),
               "--best-only", "--stop-on-zero", "--stack-diffs", "--algorithm",
               args.algorithm, str(experiment)]
    # A separate process group lets timeout/Ctrl-C stop workers and compilers too.
    process = subprocess.Popen(command, cwd=ROOT, start_new_session=True)
    try:
        return process.wait(timeout=args.seconds)
    except (subprocess.TimeoutExpired, KeyboardInterrupt) as error:
        timed_out = isinstance(error, subprocess.TimeoutExpired)
        os.killpg(process.pid, signal.SIGINT)
        try:
            process.wait(timeout=10)
        except subprocess.TimeoutExpired:
            os.killpg(process.pid, signal.SIGKILL)
            process.wait()
        print(f"Search {'time limit reached' if timed_out else 'interrupted'}; "
              f"candidates preserved in {experiment}.", flush=True)
        return 124 if timed_out else 130


def main():
    # Private compile protocol used by generated compile.sh wrappers.
    if len(sys.argv) > 1 and sys.argv[1] == "--compile":
        parser = argparse.ArgumentParser()
        parser.add_argument("--compile", type=Path, required=True)
        parser.add_argument("source", type=Path)
        parser.add_argument("-o", type=Path, required=True)
        args = parser.parse_args()
        compile_candidate(args.compile, args.source.resolve(), args.o.resolve())
        return 0

    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("function", help="function name or generated assembly path")
    parser.add_argument("-s", "--source-file", type=Path, required=True,
                        help="C attempt using real project headers")
    parser.add_argument("--unit", type=Path, help="owning repository-relative C unit")
    parser.add_argument("--permuter-dir", type=Path,
                        default=Path(os.environ.get("DECOMP_PERMUTER", ROOT.parent / "decomp-permuter")),
                        help="decomp-permuter checkout (default: ../decomp-permuter or DECOMP_PERMUTER)")
    parser.add_argument("--prepare-only", action="store_true")
    parser.add_argument("-j", "--jobs", type=int, default=2)
    parser.add_argument("--seconds", type=float, default=300,
                        help="search time limit in seconds (default: 300)")
    parser.add_argument("--algorithm", choices=["difflib", "levenshtein"], default="difflib")
    args = parser.parse_args()
    if args.jobs < 1 or not math.isfinite(args.seconds) or args.seconds <= 0:
        parser.error("jobs and seconds must be positive finite values")
    permuter = args.permuter_dir.resolve()
    for name in ("permuter.py", "strip_other_fns.py"):
        if not (permuter / name).is_file():
            parser.error(f"missing {permuter / name}; supply --permuter-dir or DECOMP_PERMUTER")
    # Check dependencies before creating an experiment.
    subprocess.run([sys.executable, str(permuter / "permuter.py"), "--help"],
                   stdout=subprocess.DEVNULL, check=True)
    experiment = prepare(args, permuter)
    return 0 if args.prepare_only else run_search(args, permuter, experiment)


if __name__ == "__main__":
    try:
        sys.exit(main())
    except (OSError, ValueError, subprocess.CalledProcessError) as error:
        print(f"Permuter failed: {error}", file=sys.stderr)
        sys.exit(2)
