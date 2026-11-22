#!/usr/bin/env python3
"""Prepare and run bounded decomp-permuter searches with the real PSX pipeline."""

import argparse
from importlib import import_module
import json
import math
import os
from pathlib import Path
import re
import shlex
import shutil
import signal
import subprocess
import sys
import tempfile

import build
from decomp_env import bootstrap, check_image, fetch_scratch, function_span, resolve_function, scratch_id

ROOT = build.ROOT


def scratch_compiler(scratch, compiler_root):
    if scratch.platform != "ps1" or scratch.compiler not in ("psyq4.1", "psyq4.4", "psyq4.5", "psyq4.6"):
        raise ValueError(f"unsupported scratch compiler: {scratch.platform}/{scratch.compiler}")
    if not isinstance(scratch.compiler_flags, str):
        raise ValueError("scratch compiler flags must be text")
    flags = shlex.split(scratch.compiler_flags)
    # Accept compiler options without allowing a scratch to redirect tools,
    # supply input files, or inject extra assembler arguments into CCPSX.
    options = re.compile(
        r"-(?:O[0-3s]?|g[0-3]?|G\d+|[fW][\w-]+"
        r"|m[\w-]+(?:=r?\d+)?|[DU]\w+(?:=[^\r\n]*)?)"
    )
    for flag in flags:
        if not options.fullmatch(flag):
            raise ValueError(f"unsupported scratch compiler flag: {flag}")
    compiler = (compiler_root / scratch.compiler).resolve()
    for filename in ("CCPSX.EXE", "CPPPSX.EXE", "CC1PSX.EXE", "ASPSX.EXE", "psyq-obj-parser"):
        if not (compiler / filename).is_file():
            raise ValueError(f"missing {compiler / filename}; supply --decompme-compilers")
    env = dict(os.environ)
    env.setdefault("WINEDEBUG", "-all")
    windows_path = subprocess.check_output(["winepath", "-w", str(compiler)], env=env, text=True).strip()
    return {"directory": str(compiler), "windows_path": windows_path, "flags": flags}


def psyq(compiler, directory, *args, stdout=None):
    compiler_path = Path(compiler["directory"])
    windows_path = compiler["windows_path"]
    (directory / "SN.INI").write_text(
        f"[ccpsx]\ncompiler_path={windows_path}\nassembler_path={windows_path}\ntmpdir=.\n"
    )
    env = dict(os.environ, SN_PATH=".")
    env.setdefault("WINEDEBUG", "-all")
    subprocess.run(["wine", str(compiler_path / "CCPSX.EXE"), *compiler["flags"], *args],
                   cwd=directory, env=env, stdout=stdout, check=True)


def compile_scratch(workspace, source, output, metadata, scratch_dir):
    original = workspace / "original.c"
    text = original.read_text()
    if source != original:
        candidate = source.read_text()
        span = function_span(candidate, metadata["function"], body_only=True)
        if span is None:
            raise ValueError(f"candidate has no definition of {metadata['function']}")
        start, end = metadata["scratch_span"]
        text = text[:start] + candidate[span[0]:span[1]] + text[end:]
    # Keep the preprocessor context, declarations, and helper functions intact.
    (scratch_dir / "code.c").write_text(text)
    compiler = metadata["scratch_compiler"]
    psyq(compiler, scratch_dir, "-c", "code.c", "-o", "output.obj")
    subprocess.run([str(Path(compiler["directory"]) / "psyq-obj-parser"), "output.obj",
                    "-o", str(output)], cwd=scratch_dir, stdout=subprocess.DEVNULL, check=True)


def compile_candidate(workspace, source, output):
    metadata = json.loads((workspace / "workspace.json").read_text())
    # Workers must never share intermediate preprocessor/assembler outputs.
    with tempfile.TemporaryDirectory(prefix="psx-permuter-") as scratch:
        scratch = Path(scratch)
        if "scratch_compiler" in metadata:
            compile_scratch(workspace, source, output, metadata, scratch)
            return
        # PERMUTER preprocessing suppresses the SDK assembler macro prelude.
        candidate = scratch / "candidate.c"
        include_path = json.dumps(str(source), ensure_ascii=False)
        candidate.write_text(f'#include "include_asm.h"\n#include {include_path}\n')
        obj = build.compile_c(candidate, True, output_root=scratch,
                              unit_path=Path(metadata["unit"]),
                              only_function=metadata["function"])
        shutil.copyfile(obj, output)


def prepare(args, permuter, scratch):
    function = args.function
    imported_source = None
    if scratch:
        imported = fetch_scratch(scratch)
        function = imported.function
        compiler = scratch_compiler(imported, args.decompme_compilers)
        span = function_span(imported.source, function, body_only=True)
        if span is None:
            raise ValueError(f"scratch source has no definition of {function}")
        imported_source = f"// Imported from https://decomp.me/scratch/{scratch}\n\n" + imported.source
    oracle = resolve_function(function, args.unit)
    source = args.source_file.resolve() if args.source_file else None
    if source is not None and not source.is_file():
        raise ValueError(f"source does not exist: {source}")
    workspace = bootstrap(oracle, None, imported_source)
    experiment = workspace / "permuter"
    experiment.mkdir()
    original = experiment / "original.c"
    metadata = json.loads((workspace / "workspace.json").read_text())
    if scratch:
        prefix = '#line 1 "ctx.c"\n' + imported.context + '\n#line 1 "src.c"\n'
        original.write_text(prefix + imported.source + "\n")
        source = original
        metadata.update(scratch_compiler=compiler,
                        scratch_span=[len(prefix) + offset for offset in span])
        print(f"Scratch {scratch} uses {imported.compiler} {shlex.join(compiler['flags'])}", flush=True)
    else:
        shutil.copyfile(source, original)
    settings = build.get_settings(oracle.unit, oracle.name, comparison=True)
    # Flatten real project headers once. PERM_* invocations survive cpp and are
    # expanded by decomp-permuter before compile.sh sees each candidate.
    cpp = [settings["CPP"], *settings["CPP_FLAGS"], "-DNON_MATCHING", "-DPERMUTER",
           "-I" + str(source.parent), "-I" + str((ROOT / oracle.unit).parent),
           "-P", str(source)]
    base = experiment / "base.c"
    if args.preserve_macros:
        sys.path.insert(0, str(permuter))
        importer = import_module("import")
        pragmas = import_module("src.ast_util")
        preserve = importer.build_preserve_macros(str(ROOT), args.preserve_macros, {})
        preprocessed, macros = importer.preprocess_c_with_macros(cpp, ROOT, preserve)
        base.write_text(preprocessed)
        print("Preserved macros: " + (", ".join(macros) or "(none found)"), flush=True)
    elif scratch:
        with tempfile.TemporaryDirectory(prefix="psx-permuter-") as temporary:
            scratch_dir = Path(temporary)
            shutil.copyfile(original, scratch_dir / "code.c")
            with base.open("w") as preprocessed:
                psyq(compiler, scratch_dir, "-E", "-P", "-DPERMUTER", "code.c", stdout=preprocessed)
    else:
        subprocess.run([*cpp, "-o", str(base)], cwd=ROOT, check=True)
    subprocess.run([sys.executable, str(permuter / "strip_other_fns.py"),
                    str(base), oracle.name], check=True)
    shutil.copyfile(workspace / "target.o", experiment / "target.o")
    (experiment / "workspace.json").write_text(json.dumps(metadata, indent=2) + "\n")
    command = shlex.join([sys.executable, str(ROOT / "tools/permuter.py"),
                          "--compile", str(experiment)])
    wrapper = experiment / "compile.sh"
    wrapper.write_text('#!/bin/sh\nset -eu\nexec ' + command + ' "$@"\n')
    wrapper.chmod(0o755)
    settings_text = (
        'compiler_type = "gcc"\nfunc_name = ' + json.dumps(oracle.name) +
        '\nobjdump_command = ' + json.dumps(shlex.join(
            [settings["CROSS"] + "objdump", "-drz", "-m", "mips:3000",
             f"--disassemble={oracle.name}"])) + '\n')
    if scratch:
        # These passes edit declarations or add helpers outside the scored body.
        settings_text += (
            "\n[weight_overrides]\nperm_randomize_external_type = 0.0\n"
            "perm_randomize_function_type = 0.0\nperm_inline = 0.0\n"
        )
    (experiment / "settings.toml").write_text(settings_text)
    # Smoke-test the exact wrapper used by workers before starting a search.
    smoke_source = base
    if args.preserve_macros:
        smoke_source = experiment / "smoke.c"
        smoke_source.write_text(pragmas.process_pragmas(base.read_text()))
    subprocess.run([str(wrapper), str(smoke_source), "-o", str(experiment / "base.o")],
                   cwd=ROOT, check=True)
    print(f"Prepared experiment: {experiment}", flush=True)
    print("Scores are search hints. Check winners with tools/match_function.py; "
          "integration still requires the full-image gate.", flush=True)
    return experiment


def run_search(args, permuter, experiment):
    command = [sys.executable, str(permuter / "permuter.py"), "-j", str(args.jobs),
               "--best-only", "--stop-on-zero", "--stack-diffs", "--algorithm",
               args.algorithm]
    if args.quiet:
        command.append("--quiet")
    command.append(str(experiment))
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

    parser = argparse.ArgumentParser(prog="tools/permuter", description=__doc__)
    parser.add_argument("function", help="function name, generated assembly path, or decomp.me scratch ID/URL")
    parser.add_argument("-s", "--source-file", type=Path,
                        help="C attempt for a local function or assembly path")
    parser.add_argument("--unit", type=Path, help="owning repository-relative C unit")
    parser.add_argument("--permuter-dir", type=Path,
                        default=os.environ.get("DECOMP_PERMUTER"),
                        help="decomp-permuter checkout (overrides DECOMP_PERMUTER and checkout discovery)")
    parser.add_argument("--decompme-compilers", type=Path, default=os.environ.get(
        "DECOMP_ME_COMPILERS", ROOT.parent / "decomp.me/backend/compilers/ps1"),
        help="directory containing decomp.me's Psy-Q toolchains (or DECOMP_ME_COMPILERS)")
    parser.add_argument("--prepare-only", action="store_true")
    parser.add_argument("--preserve-macros", metavar="REGEX",
                        help="preserve matching SDK macros, for example 'gte_.*'")
    parser.add_argument("--quiet", action="store_true", help="suppress search updates")
    parser.add_argument("-j", "--jobs", type=int, default=2)
    parser.add_argument("--seconds", type=float, default=300,
                        help="search time limit in seconds (default: 300)")
    parser.add_argument("--algorithm", choices=["difflib", "levenshtein"], default="difflib")
    args = parser.parse_args()
    if args.jobs < 1 or not math.isfinite(args.seconds) or args.seconds <= 0:
        parser.error("jobs and seconds must be positive finite values")
    if args.preserve_macros:
        try:
            re.compile(f"^(?:{args.preserve_macros})$")
        except re.error as error:
            parser.error(f"invalid --preserve-macros regex: {error}")
    checkouts = [ROOT / "tools/decomp-permuter", ROOT.parent / "decomp-permuter",
                 ROOT.parent / "snowboardkids-decomp/tools/decomp-permuter"]
    required_scripts = ("permuter.py", "strip_other_fns.py")
    permuter = (args.permuter_dir or next(
        (path for path in checkouts if all((path / name).is_file() for name in required_scripts)),
        checkouts[1],
    )).resolve()
    for name in required_scripts:
        if not (permuter / name).is_file():
            parser.error(f"missing {permuter / name}; supply --permuter-dir or DECOMP_PERMUTER")
    # Check dependencies before creating an experiment.
    subprocess.run([sys.executable, str(permuter / "permuter.py"), "--help"],
                   stdout=subprocess.DEVNULL, check=True)
    check_image()
    scratch = scratch_id(args.function) if args.source_file is None else None
    if not scratch and args.source_file is None:
        parser.error("local functions and assembly paths require -s/--source-file")
    experiment = prepare(args, permuter, scratch)
    return 0 if args.prepare_only else run_search(args, permuter, experiment)


if __name__ == "__main__":
    try:
        sys.exit(main())
    except (OSError, ValueError, subprocess.CalledProcessError) as error:
        print(f"Permuter failed: {error}", file=sys.stderr)
        sys.exit(2)
