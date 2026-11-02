#!/usr/bin/env python3
"""Create and run an isolated PSX function-matching workspace."""

import argparse
import fcntl
import hashlib
import json
import re
import shutil
import subprocess
import sys
from pathlib import Path

import build
from decomp import ROOT, Elf32, PsxImage, active_functions, parse_asm, require_fresh, uncomment
from match_function import owning_unit
from validate_commit import verify_image

TEMPLATES = ROOT / "tools/decomp-env"


def check_image():
    require_fresh()
    verify_image(ROOT)


def resolve_function(value, requested_unit):
    functions = list(active_functions(sdk=True))
    if value.endswith(".s"):
        supplied = Path(value).resolve()
        name = parse_asm(supplied).name
    else:
        supplied = None
        name = value
    if not re.fullmatch(r"[A-Za-z_]\w*", name):
        raise ValueError("provide a function name or its generated assembly path")
    unit = requested_unit or owning_unit(name, functions)
    if unit.is_absolute() or ".." in unit.parts or not (ROOT / unit).is_file():
        raise ValueError("--unit must identify a repository-relative C source")
    oracle = next((f for f in functions if f.name == name and f.unit == unit), None)
    if oracle is None:
        path = ROOT / "asm/matchings" / unit.relative_to("src").with_suffix("") / (name + ".s")
        oracle = parse_asm(path, unit, name)
    if supplied and supplied != oracle.path.resolve():
        raise ValueError("assembly path must identify the function's current generated oracle")
    original = PsxImage((ROOT / "disc/SLPS_018.23").read_bytes())
    if original.read(oracle.address, len(oracle.data)) != oracle.data:
        raise ValueError("assembly differs from the original executable; rebuild first")
    return oracle


def assemble_target(workspace, oracle):
    source = workspace / "target.s"
    source.write_text('.include "include/macro.inc"\n.set noreorder\n.set noat\n.text\n' + oracle.path.read_text())
    settings = build.get_settings(oracle.unit)
    flags = settings["ORACLE_AS_FLAGS"]
    obj = workspace / "target.o"
    subprocess.run([build.AS, *flags, "-o", str(obj), str(source)], cwd=ROOT, check=True)
    reference = Elf32(ROOT / "build/slps_018.23.elf")
    assignments = {"_gp": reference.symbols["_gp"].address}
    for symbol in Elf32(obj).symbols.values():
        if symbol.section == 0:
            if symbol.name not in reference.symbols:
                raise ValueError(f"target has an unresolved reference: {symbol.name}")
            assignments[symbol.name] = reference.symbols[symbol.name].address
    script = workspace / "target.ld"
    script.write_text(f'SECTIONS {{ .text 0x{oracle.address:X} : SUBALIGN(4) {{ *(.text) }} '
                      '/DISCARD/ : { *(.reginfo) *(.MIPS.abiflags) *(.pdr) *(.gnu.attributes) } }\n' +
                      "\n".join(f"{name} = 0x{value:X};" for name, value in assignments.items()))
    elf = workspace / "target.elf"
    subprocess.run([build.LD, "-EL", "-T", str(script), "-o", str(elf), str(obj)], check=True)
    target = Elf32(elf)
    text = target.section_data(target.section_names.index(".text"))
    if text != oracle.data:
        raise ValueError("assembled target does not reproduce the oracle bytes")
    (workspace / "target.bin").write_bytes(text)


def existing_body(source, name):
    source = uncomment(source)
    start = re.search(r"^[^\n;{}#]*\b" + re.escape(name) + r"\s*\([^;{}]*\)\s*\{", source, re.M)
    if start is None:
        return None
    depth = 1
    tokens = r'"(?:\\.|[^"\\])*"|\'(?:\\.|[^\'\\])*\'|[{}]'
    for token in re.finditer(tokens, source[start.end():]):
        if token[0] == "{":
            depth += 1
        elif token[0] == "}":
            depth -= 1
            if depth == 0:
                return source[start.start():start.end() + token.end()] + "\n"
    raise ValueError(f"cannot find end of {name}")


def seed_source(workspace, oracle):
    source = ROOT / oracle.unit
    original = source.read_text()
    includes = "\n".join(re.findall(r'^\s*#include[^\n]+', original, re.M)).strip() + "\n\n"
    context = workspace / "context.c"
    subprocess.run([build.CPP, *build.CPP_FLAGS, "-DM2CTX", "-DNON_MATCHING", "-P",
                    str(source), "-o", str(context)], cwd=ROOT, check=True)
    body = existing_body(original, oracle.name)
    if body is None:
        m2c = shutil.which("m2c")
        if m2c:
            result = subprocess.run([m2c, "--target", "mipsel-gcc-c", "--context", str(context),
                                     str(oracle.path)], cwd=ROOT, text=True, capture_output=True)
            (workspace / "m2c.log").write_text(result.stderr)
            if result.returncode == 0:
                body = result.stdout
            else:
                print("m2c could not seed base.c; see m2c.log.")
        else:
            print("m2c is unavailable; base.c contains the owning unit's includes.")
    (workspace / "base.c").write_text(includes + (body or ""))


def bootstrap(oracle, identifier):
    stem = oracle.name
    if identifier is not None:
        if not re.fullmatch(r"[A-Za-z0-9_-]+", identifier):
            raise ValueError("--id accepts only letters, digits, underscores, and hyphens")
        stem += "-" + identifier
    workspace = ROOT / "nonmatchings" / stem
    metadata = {"function": oracle.name, "unit": oracle.unit.as_posix(),
                "address": oracle.address, "target_sha1": hashlib.sha1(oracle.data).hexdigest()}
    if identifier is not None and workspace.exists():
        if json.loads((workspace / "workspace.json").read_text()) != metadata:
            raise ValueError("existing workspace targets a different function or byte range; choose another --id")
        return workspace
    suffix = 2
    while workspace.exists():
        workspace = ROOT / "nonmatchings" / f"{stem}-{suffix}"
        suffix += 1
    workspace.mkdir(parents=True)
    try:
        assemble_target(workspace, oracle)
        seed_source(workspace, oracle)
        (workspace / "project").symlink_to(ROOT, target_is_directory=True)
        for name in ["build.sh", "diff.sh"]:
            (workspace / name).symlink_to(TEMPLATES / name)
        instructions = (TEMPLATES / "AGENTS.md").read_text()
        instructions = instructions.replace("$functionName", oracle.name).replace("$unit", oracle.unit.as_posix())
        (workspace / "AGENTS.md").write_text(instructions)
        (workspace / "workspace.json").write_text(json.dumps(metadata, indent=2) + "\n")
    except Exception:
        shutil.rmtree(workspace)
        raise
    return workspace


def build_attempt(workspace, source):
    with (workspace / ".build.lock").open("a") as lock:
        fcntl.flock(lock, fcntl.LOCK_EX)
        return compare_attempt(workspace, source)


def compare_attempt(workspace, source):
    check_image()
    metadata = json.loads((workspace / "workspace.json").read_text())
    oracle = resolve_function(metadata["function"], Path(metadata["unit"]))
    if hashlib.sha1(oracle.data).hexdigest() != metadata["target_sha1"]:
        raise ValueError("workspace target changed; bootstrap a new workspace")
    source = Path(source)
    if not source.is_absolute():
        source = workspace / source
    source = source.resolve()
    if not source.is_file() or source.suffix != ".c":
        raise ValueError("provide an existing C attempt")
    latest = workspace / "latest.json"
    latest.unlink(missing_ok=True)
    output = workspace / "attempts" / source.stem
    suffix = 2
    while output.exists():
        output = workspace / "attempts" / f"{source.stem}-{suffix}"
        suffix += 1
    result = subprocess.run([sys.executable, str(ROOT / "tools/match_function.py"), oracle.name,
                             str(source), "--unit", str(oracle.unit), "--output", str(output)], cwd=ROOT)
    if result.returncode in (0, 1) and (output / "result.json").is_file():
        latest.write_text(json.dumps({"output": str(output)}, indent=2) + "\n")
        return result.returncode
    return 2


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("function", nargs="?", help="function name or generated assembly path")
    parser.add_argument("--bootstrap", action="store_true", help="create a workspace before launching Codex (default)")
    parser.add_argument("--bootstrap-only", action="store_true", help="create the workspace without launching Codex")
    parser.add_argument("--id", help="reuse nonmatchings/FUNCTION-ID without overwriting attempts")
    parser.add_argument("--unit", type=Path, help="owning repository-relative C unit if ambiguous")
    parser.add_argument("--workspace", type=Path, help=argparse.SUPPRESS)
    mode = parser.add_mutually_exclusive_group()
    mode.add_argument("--build", metavar="SOURCE", help=argparse.SUPPRESS)
    mode.add_argument("--diff", action="store_true", help=argparse.SUPPRESS)
    args = parser.parse_args()
    if args.workspace:
        workspace = args.workspace.resolve()
        if not workspace.is_relative_to(ROOT / "nonmatchings"):
            raise ValueError("workspaces must be inside nonmatchings/")
        if args.build:
            return build_attempt(workspace, args.build)
        if args.diff:
            latest = json.loads((workspace / "latest.json").read_text())
            print((Path(latest["output"]) / "diff.txt").read_text(), end="")
            return 0
    if args.build or args.diff or not args.function:
        parser.error("provide a function, or use build.sh/diff.sh inside a workspace")
    check_image()
    oracle = resolve_function(args.function, args.unit)
    workspace = bootstrap(oracle, args.id)
    print(f"Environment ready: {workspace.relative_to(ROOT)}", flush=True)
    if args.bootstrap_only:
        return 0
    codex = shutil.which("codex")
    if codex is None:
        print(f"Codex is unavailable; work in {workspace} using ./build.sh base.c")
        return 0
    prompt = f"Match {oracle.name}. The workspace is ready; use AGENTS.md, target.s, and ./build.sh."
    return subprocess.run([codex, prompt], cwd=workspace).returncode


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (ValueError, OSError, KeyError, subprocess.CalledProcessError) as error:
        print(f"Workspace error: {error}", file=sys.stderr)
        raise SystemExit(2)
