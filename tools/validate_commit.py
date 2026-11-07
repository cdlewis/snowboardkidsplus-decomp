#!/usr/bin/env python3
"""Validate staged PSX changes without hiding or modifying unstaged work."""

import ast
import hashlib
import os
import re
import subprocess
import sys
from pathlib import Path

GENERATED = ("asm/", "build/", "expected/", "disc/")


def git(root, *args):
    return subprocess.check_output(["git", *args], cwd=root)


def paths(output):
    return [os.fsdecode(p) for p in output.split(b"\0") if p]


def build_input(path):
    return (path in {"Makefile", "snowboardkidsplus.yaml", "decomp.yaml",
                         "symbol_addrs.txt", "reloc_addrs.txt"}
            or path.startswith(("src/", "include/", "tools/", "config/")))


def checksum(path, data):
    source = data.decode("utf-8")
    if path in ("tools/build.py", "tools/snb_dat.py"):
        name = "TARGET_SHA1" if path == "tools/build.py" else "DAT_SHA1"
        for node in ast.parse(source).body:
            if isinstance(node, ast.Assign) and any(
                    isinstance(t, ast.Name) and t.id == name for t in node.targets):
                return ast.literal_eval(node.value)
        raise ValueError(f"{path} must retain {name}")
    values = re.findall(r"^\s*sha1:\s*([0-9a-fA-F]{40})\s*$", source, re.M)
    if len(values) != 1:
        raise ValueError(f"expected one SHA1 in {path}")
    return values[0].lower()


def check_inputs(root):
    if git(root, "ls-files", "--unmerged", "-z"):
        raise ValueError("resolve the unmerged index before validation")
    staged = paths(git(root, "diff", "--cached", "--name-only", "-z"))
    generated = [p for p in staged if p.startswith(GENERATED)
                 or (p.startswith("linkers/") and
                     (p.endswith(".ld") or Path(p).name.startswith("undefined_")))]
    if generated:
        raise ValueError("generated outputs must not be staged: " + ", ".join(generated))
    dirty = paths(git(root, "diff", "--no-ext-diff", "--name-only", "-z"))
    untracked = paths(git(root, "ls-files", "--others", "--exclude-standard", "-z"))
    pending = sorted({p for p in dirty + untracked if build_input(p)})
    if pending:
        raise ValueError("build inputs differ from the index; stage or restore these files "
                         "before committing:\n  " + "\n  ".join(pending))
    for path in ("tools/build.py", "snowboardkidsplus.yaml", "decomp.yaml"):
        before = checksum(path, git(root, "show", f"HEAD:{path}"))
        after = checksum(path, git(root, "show", f":{path}"))
        if before != after:
            raise ValueError(f"expected checksum changed in {path}; validation cannot accept a new target")
    dat_source = git(root, "show", ":tools/snb_dat.py")
    if checksum("tools/snb_dat.py", dat_source) != committed_dat_sha1(root):
        raise ValueError("expected DAT checksum changed; validation cannot accept a new target")
    return git(root, "write-tree").strip()


def verify_image(root):
    # Use the committed checksum independently of build.py's verification.
    expected = checksum("tools/build.py", git(root, "show", "HEAD:tools/build.py"))
    for path in ("disc/SLPS_018.23", "build/slps_018.23.exe"):
        actual = hashlib.sha1((root / path).read_bytes()).hexdigest()
        if actual != expected:
            raise ValueError(f"{path}: SHA1 {actual} differs from protected checksum {expected}")
    # Independently verify the complete image, including its size and header.
    if (root / "disc/SLPS_018.23").read_bytes() != (root / "build/slps_018.23.exe").read_bytes():
        raise ValueError("rebuilt executable differs from disc/SLPS_018.23")


def committed_dat_sha1(root):
    exists = subprocess.run(["git", "cat-file", "-e", "HEAD:tools/snb_dat.py"], cwd=root,
                            stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL).returncode == 0
    if exists:
        return checksum("tools/snb_dat.py", git(root, "show", "HEAD:tools/snb_dat.py"))
    # Bootstrap protection until the DAT pipeline's first commit.
    return "226b3cd476010a340455b9720b2e37dcd94b7933"


def verify_dat(root):
    expected = committed_dat_sha1(root)
    original = (root / "disc/SNB/SNB.DAT").read_bytes()
    rebuilt = (root / "build/dat/SNB.DAT").read_bytes()
    for name, data in (("disc/SNB/SNB.DAT", original), ("build/dat/SNB.DAT", rebuilt)):
        if hashlib.sha1(data).hexdigest() != expected:
            raise ValueError(f"{name}: DAT SHA1 differs from protected checksum {expected}")
    if rebuilt != original:
        raise ValueError("rebuilt DAT differs from disc/SNB/SNB.DAT")


def validate(root):
    from build_and_verify import build_and_verify

    tree = check_inputs(root)
    build_and_verify(root)
    verify_image(root)
    verify_dat(root)
    if check_inputs(root) != tree:
        raise ValueError("the staged snapshot changed during validation; retry the commit")
    print("[pre-commit] Staged inputs validated; full executable matches.", flush=True)


def main():
    root = Path(subprocess.check_output(
        ["git", "rev-parse", "--show-toplevel"], text=True).strip())
    validate(root)


if __name__ == "__main__":
    try:
        main()
    except (ValueError, OSError, SyntaxError, subprocess.CalledProcessError) as error:
        print(f"[pre-commit] Validation failed: {error}", file=sys.stderr)
        raise SystemExit(1)
