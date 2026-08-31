#!/usr/bin/env python3
"""Snowboard Kids Plus (SLPS-01823) matching-decomp build driver.

Modeled on NFSHS-PSX-decomp's tools/build.py, trimmed to this project's
needs and Linux toolchain.

Pipeline (PSY-Q-faithful):

    src/*.c  --cpp-->  *.i  --cc1 (real gcc 2.x PSX build)-->  *.s
             --maspsx--> GNU as --> ELF .o
    asm/*.s  --GNU as--> ELF .o
    all .o   --GNU ld (linkers/slps_018.23.ld)--> slps_018.23.elf
             --objcopy--> flat PS-EXE, sha1-compared to disc/SLPS_018.23

Usage:
    python tools/build.py                  # full build + verify
    python tools/build.py --skip-asm       # -DSKIP_ASM C objects (objdiff base)
    python tools/build.py --update-expected  # snapshot build objs to expected/
    python tools/build.py clean

The compiler version is not yet pinned (candidates vendored under
tools/gcc/). Override with SKP_CC1=<path to cc1>.
"""

import hashlib
import os
import shutil
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
OUT = ROOT / "build"

BASENAME = "slps_018.23"
TARGET = ROOT / "disc" / "SLPS_018.23"
TARGET_SHA1 = "3f1a7abc267ea29bafed1653121613ea467c1ac8"


def _env(name, default):
    return Path(os.environ.get(name, default))


PY = sys.executable
CROSS = os.environ.get("SKP_CROSS", "mips-linux-gnu-")
AS = CROSS + "as"
LD = CROSS + "ld"
OBJCOPY = CROSS + "objcopy"
CPP = os.environ.get("SKP_CPP", "cpp")  # host cpp is fine for preprocessing
CC1 = _env("SKP_CC1", ROOT / "tools" / "gcc" / "gcc-2.8.1-psx" / "cc1")
MASPSX = _env("SKP_MASPSX", ROOT / "tools" / "maspsx" / "maspsx.py")
ASPSX_VERSION = os.environ.get("SKP_ASPSX_VERSION", "2.77")

G_VALUE = os.environ.get("SKP_G_VALUE", "0")  # revisit once sdata is mapped

CPP_FLAGS = [
    "-nostdinc", "-undef", "-D__GNUC__=2", "-D__OPTIMIZE__",
    "-Dmips", "-D__mips__", "-D__psx__", "-DPSX",
    "-DINCLUDE_ASM_USE_MACRO_INC=1",
    f"-I{ROOT / 'include'}",
]
CC1_FLAGS = ["-quiet", "-O2", f"-G{G_VALUE}", "-fgnu-linker", "-mcpu=3000"]
# -I paths let GNU as resolve `.include "../asm/..."` / `.include
# "../include/macro.inc"` (emitted relative to src/) and plain "macro.inc".
AS_FLAGS = ["-EL", "-march=r3000", "-mtune=r3000", "-no-pad-sections",
            f"-G{G_VALUE}", "-I", str(ROOT / "include"), "-I", str(ROOT / "src"),
            "-I", str(ROOT / "build")]

# Per-TU overrides, NFSHS-style (flags proven per translation unit go here).
PER_TU_FLAGS: dict = {}


def run(cmd, **kw):
    return subprocess.run([str(c) for c in cmd], capture_output=True,
                          text=True, **kw)


def compile_c(src: Path, skip_asm: bool) -> Path:
    """cpp -> cc1 -> maspsx -> as => build/<rel>.o (path from linker script)."""
    rel = src.relative_to(ROOT)
    obj = OUT / rel.with_suffix(".o")
    obj.parent.mkdir(parents=True, exist_ok=True)
    i_file = obj.with_suffix(".i")
    s_file = obj.with_suffix(".s")

    cpp = [CPP, *CPP_FLAGS]
    if skip_asm:
        cpp.append("-DSKIP_ASM")
    cpp += [src, "-o", i_file]
    r = run(cpp)
    if r.returncode:
        sys.exit(f"[cpp] {rel}\n{r.stderr}")

    r = run([CC1, *CC1_FLAGS, i_file, "-o", s_file])
    if r.returncode:
        sys.exit(f"[cc1] {rel}\n{r.stdout}{r.stderr}")

    r = run([PY, MASPSX, f"--aspsx-version={ASPSX_VERSION}", "--expand-div"],
            input=s_file.read_text(), cwd=ROOT)
    if r.returncode:
        sys.exit(f"[maspsx] {rel}\n{r.stderr}")
    masp_s = obj.with_suffix(".masp.s")
    masp_s.write_text(r.stdout)

    r = run([AS, *AS_FLAGS, "-o", obj, masp_s], cwd=ROOT)
    if r.returncode:
        sys.exit(f"[as] {rel}\n{r.stderr}")
    return obj


def assemble_s(src: Path) -> Path:
    rel = src.relative_to(ROOT)
    obj = OUT / rel.with_suffix(".o")
    obj.parent.mkdir(parents=True, exist_ok=True)
    r = run([AS, *AS_FLAGS, "-o", obj, src], cwd=ROOT)
    if r.returncode:
        sys.exit(f"[as] {rel}\n{r.stderr}")
    return obj


def link_and_verify():
    elf = OUT / f"{BASENAME}.elf"
    exe = OUT / f"{BASENAME}.exe"
    r = run([LD, "-EL", "-nostdlib",
             "-T", ROOT / "linkers" / f"{BASENAME}.ld",
             "-T", ROOT / "linkers" / "undefined_funcs_auto.txt",
             "-T", ROOT / "linkers" / "undefined_syms_auto.txt",
             "-Map", OUT / f"{BASENAME}.map", "-o", elf])
    if r.returncode:
        sys.exit(f"[ld]\n{r.stderr}")
    r = run([OBJCOPY, "-O", "binary", elf, exe])
    if r.returncode:
        sys.exit(f"[objcopy]\n{r.stderr}")
    got = hashlib.sha1(exe.read_bytes()).hexdigest()
    if got == TARGET_SHA1:
        print(f"{exe.relative_to(ROOT)}: OK")
    else:
        sys.exit(f"{exe.relative_to(ROOT)}: FAILED\n  built  {got}\n"
                 f"  target {TARGET_SHA1}")


def update_expected():
    """Snapshot build/src objects as objdiff targets (run from a verified
    all-asm or byte-matching build)."""
    src_objs = sorted((OUT / "src").rglob("*.o"))
    if not src_objs:
        sys.exit("no build/src objects; run a full build first")
    for obj in src_objs:
        dst = ROOT / "expected" / obj.relative_to(OUT)
        dst.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(obj, dst)
        print(f"expected/{obj.relative_to(OUT)}")


def main():
    args = sys.argv[1:]
    if "clean" in args:
        shutil.rmtree(OUT, ignore_errors=True)
        return
    skip_asm = "--skip-asm" in args

    for src in sorted((ROOT / "src").rglob("*.c")):
        compile_c(src, skip_asm)
        print(f"[cc] {src.relative_to(ROOT)}")
    if skip_asm:
        return  # objdiff base objects only; no link

    for src in sorted((ROOT / "asm").rglob("*.s")):
        if "nonmatchings" in src.parts:
            continue  # pulled in via INCLUDE_ASM
        assemble_s(src)
    link_and_verify()

    if "--update-expected" in args:
        update_expected()


if __name__ == "__main__":
    main()
