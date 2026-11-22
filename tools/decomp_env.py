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
from dataclasses import dataclass
from pathlib import Path
from urllib.error import URLError
from urllib.request import Request, urlopen

import build
from decomp import ROOT, Elf32, PsxImage, active_functions, parse_asm, require_fresh, uncomment
from match_function import owning_unit
from m2ctx import generate_context
from progress_attempts import inline_source
from validate_commit import verify_image

TEMPLATES = ROOT / "tools/decomp-env"


@dataclass
class Scratch:
    function: str
    source: str
    context: str
    platform: str
    compiler: str
    compiler_flags: str


def scratch_id(value):
    url = re.fullmatch(r"https://(?:www\.)?decomp\.me/scratch/([A-Za-z0-9]+)/?(?:[?#].*)?", value)
    if url:
        return url[1]
    # Prefer local symbols over the sibling wrapper's scratch-ID heuristic.
    if re.fullmatch(r"[0-9]+|[A-Za-z0-9]{5,10}", value):
        if value not in Elf32(ROOT / "build/slps_018.23.elf").symbols:
            return value
    return None


def fetch_scratch(identifier):
    headers = {"User-Agent": "snowboard-kids-plus-decomp"}
    secret = ROOT / ".decomp_dev_secret"
    if secret.is_file():
        lines = secret.read_text().splitlines()
        if lines and lines[0].strip():
            headers["User-Agent"] = lines[0].strip()
    request = Request(f"https://decomp.me/api/scratch/{identifier}", headers=headers)
    try:
        with urlopen(request, timeout=30) as response:
            scratch = json.load(response)
    except (URLError, ValueError) as error:
        raise ValueError(f"could not fetch decomp.me scratch {identifier}: {error}") from error
    if not isinstance(scratch, dict):
        raise ValueError("decomp.me returned an invalid scratch")
    label = scratch.get("diff_label")
    source = scratch.get("source_code")
    if not isinstance(label, str) or not re.fullmatch(r"[A-Za-z_]\w*(?:\.s)?", label):
        raise ValueError("decomp.me scratch has no valid function diff_label")
    if not isinstance(source, str) or not source.strip():
        raise ValueError("decomp.me scratch has no source_code")
    context = scratch.get("context")
    if context is not None and not isinstance(context, str):
        raise ValueError("decomp.me scratch has an invalid context")
    return Scratch(label.removesuffix(".s"), source.replace("\r\n", "\n"),
                   (context or "").replace("\r\n", "\n"),
                   scratch.get("platform", ""), scratch.get("compiler", ""),
                   scratch.get("compiler_flags", ""))


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


def function_span(source, name, *, body_only=False):
    masked = uncomment(source, preserve_offsets=True)
    start = re.search(r"^[^\n;{}#]*\b" + re.escape(name) + r"\s*\([^;{}]*\)\s*\{", masked, re.M)
    if start is None:
        return None
    depth = 1
    tokens = r'"(?:\\.|[^"\\])*"|\'(?:\\.|[^\'\\])*\'|[{}]'
    for token in re.finditer(tokens, masked[start.end():]):
        if token[0] == "{":
            depth += 1
        elif token[0] == "}":
            depth -= 1
            if depth == 0:
                first = start.end() - 1 if body_only else start.start()
                return first, start.end() + token.end()
    raise ValueError(f"cannot find end of {name}")


def existing_body(source, name):
    source = uncomment(source)
    span = function_span(source, name)
    if span is not None:
        return source[span[0]:span[1]] + "\n"


def seed_source(workspace, oracle, imported_source=None):
    source = ROOT / oracle.unit
    original = source.read_text()
    includes = "\n".join(re.findall(r'^\s*#include[^\n]+', original, re.M)).strip() + "\n\n"
    context = workspace / "context.c"
    context.write_text(generate_context(source))
    body = imported_source
    if body is None:
        body = inline_source(original, oracle.name) or existing_body(original, oracle.name)
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


def bootstrap(oracle, identifier, imported_source=None):
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
        seed_source(workspace, oracle, imported_source)
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


def record_attempt(workspace, output, record):
    (output / "attempt.json").write_text(json.dumps(record, indent=2) + "\n")
    score = record.get("instruction_similarity_percent")
    status = "exact" if record.get("exact") else "different" if score is not None else "error"
    score_text = f"{score:.2f}%" if score is not None else "unscored"
    with (workspace / "match_log.txt").open("a") as log:
        log.write(f'{record["attempt"]}.c {score_text} {status} '
                  f'source={record["source"]} sha1={record["source_sha1"]}\n')
    records = sorted((json.loads(path.read_text()) for path in
                      (workspace / "attempts").glob("base_*/attempt.json")),
                     key=lambda item: item["number"])
    scored = [item for item in records if "instruction_similarity_percent" in item]
    if not scored:
        return
    # Exact bytes outrank rounded similarity; ties retain the earliest attempt.
    best = max(scored, key=lambda item: (item["exact"], item["instruction_similarity_percent"]))
    (workspace / "best.json").write_text(json.dumps(best, indent=2) + "\n")
    print(f'Best: {best["attempt"]}.c — {best["instruction_similarity_percent"]:.2f}% '
          f'({"EXACT" if best["exact"] else "different bytes"}); source: {best["source"]}', flush=True)
    since = sum(item["number"] > best["number"] for item in records)
    if since >= 20 and not best["exact"]:
        print(f"STOP: No improvement in {since} attempts. Preserve the best attempt and report "
              "its score/source, approaches tried, remaining differences, and next strategy.", flush=True)


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
    attempts = workspace / "attempts"
    numbers = [int(match[1]) for path in attempts.glob("base_*")
               if (match := re.fullmatch(r"base_(\d+)", path.name))]
    number = max(numbers, default=0) + 1
    while (workspace / f"base_{number}.c").exists() and source != workspace / f"base_{number}.c":
        number += 1
    label = f"base_{number}"
    numbered_source = workspace / f"{label}.c"
    if source != numbered_source:
        shutil.copyfile(source, numbered_source)
    output = attempts / label
    output.mkdir(parents=True)
    snapshot = output / f"{label}.c"
    snapshot.write_bytes(numbered_source.read_bytes())
    source_hash = hashlib.sha1(snapshot.read_bytes()).hexdigest()
    record = {"attempt": label, "number": number, "input": str(source),
              "source": str(snapshot.relative_to(workspace)), "source_sha1": source_hash,
              "output": str(output)}
    print(f"Attempt: {label}.c", flush=True)
    result = subprocess.run([sys.executable, str(ROOT / "tools/match_function.py"), oracle.name,
                             str(numbered_source), "--unit", str(oracle.unit), "--output", str(output)], cwd=ROOT)
    if result.returncode in (0, 1) and (output / "result.json").is_file():
        report = json.loads((output / "result.json").read_text())
        if report["source_sha1"] != source_hash:
            record_attempt(workspace, output, record)
            raise ValueError("attempt source changed during compilation; score was not logged")
        record.update({key: report[key] for key in ("exact", "instruction_similarity_percent",
                                                   "matching_instruction_words", "comparison_words")})
        record_attempt(workspace, output, record)
        latest.write_text(json.dumps({"output": str(output)}, indent=2) + "\n")
        return result.returncode
    record_attempt(workspace, output, record)
    return 2


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("function", nargs="?", help="function name, generated assembly path, or decomp.me scratch ID/URL")
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
    imported_source = None
    function = args.function
    scratch = scratch_id(function)
    if scratch:
        if args.id is not None:
            parser.error("--id cannot be used when importing a decomp.me scratch")
        imported = fetch_scratch(scratch)
        function = imported.function
        imported_source = f"// Imported from https://decomp.me/scratch/{scratch}\n\n" + imported.source
    oracle = resolve_function(function, args.unit)
    workspace = bootstrap(oracle, args.id, imported_source)
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
