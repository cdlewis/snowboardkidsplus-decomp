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
import re
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
PER_TU_FLAGS: dict = {
    "src/main_remaining.c": {"g_value": "8"},
    "src/engine/game_options.c": {"g_value": "8"},
    # Proven against the RNG function bytes; the game-wide compiler and -G
    # setting are still unpinned. Keep these choices local to the carved units.
    "src/engine/random.c": {
        "g_value": "8",
        # Same compiler for the entire game. The wrapper needs an unfilled
        # return delay slot; the other four functions need normal scheduling.
        "function_flags": {"resetGameplayRng": ["-fno-delayed-branch"]},
    },
}


# Verified SDK imports only: preserve assembler-scheduled slots and object padding.
# This lane is independent of the single compiler used by game C.
SDK_CC1 = ROOT / "tools/gcc/gcc-2.7.2-psx/cc1"
for _sdk_unit in (
    "libsnd/vs_mono", "libsnd/vm_doff", "libsnd/vm_don",
    "libspu/s_it", "libspu/s_stm", "libetc/vmode",
    "libspu/s_snv", "libspu/s_srv", "libsnd/ssstart", "libpress/libpress",
    "libetc/intr", "libapi/counter",
    "libsnd/ssclose", "libsnd/ssstop", "libsnd/ssplay", "libsnd/vs_vh", "libsnd/vs_vtc",
    "libgpu/ext", "libgpu/sys", "libgpu/p00", "libgpu/p01", "libgs/gs_122",
):
    PER_TU_FLAGS[f"src/syslib/{_sdk_unit}.c"] = {
        "cc1": SDK_CC1, "g_value": "0", "gas_reorder": True,
        "pad_sections": True,
    }

PER_TU_FLAGS["src/syslib/libgpu/sys.c"]["cc1_flags"] = ["-fno-builtin"]

PER_TU_FLAGS["src/syslib/libcd/sys.c"] = {
    "gas_reorder": True,
    "function_flags": {
        name: ["-fno-delayed-branch"]
        for name in ("CdFlush", "CdSync", "CdReady", "CdDataSync")
    }
}
PER_TU_FLAGS["src/syslib/libcd/sys.c"]["function_flags"].update({
    name: ["-mno-split-addresses"]
    for name in ("CdSetDebug", "CdSyncCallback", "CdReadyCallback")
})


def run(cmd, **kw):
    return subprocess.run([str(c) for c in cmd], capture_output=True,
                          text=True, **kw)


def replace_compiler_function(normal: str, alternate: str, name: str) -> str:
    """Select one complete .ent/.end region from the same compiler's flag run.

    No instructions are rewritten. The original data, declarations, and other
    functions remain in the normal compilation. The RNG wrapper has no local
    labels, so moving its region cannot collide with another function's labels.
    """
    symbol = re.escape(name)
    pattern = re.compile(
        rf"^[ \t]*\.ent[ \t]+{symbol}[ \t]*\n"
        rf".*?^[ \t]*\.end[ \t]+{symbol}[ \t]*(?:\n|$)",
        re.MULTILINE | re.DOTALL,
    )
    original = list(pattern.finditer(normal))
    replacement = list(pattern.finditer(alternate))
    if len(original) != 1 or len(replacement) != 1:
        raise ValueError(f"expected one compiler function region for {name}")
    start, end = original[0].span()
    return normal[:start] + replacement[0].group() + normal[end:]


def compile_c(src: Path, skip_asm: bool, *, output_root: Path = None,
              unit_path: Path = None, only_function: str = None,
              extra_flags=()) -> Path:
    """cpp -> cc1 -> maspsx -> as => build/<rel>.o (path from linker script)."""
    rel = unit_path if unit_path is not None else src.relative_to(ROOT)
    overrides = PER_TU_FLAGS.get(rel.as_posix(), {})
    cc1 = overrides.get("cc1", CC1)
    g_value = overrides.get("g_value", G_VALUE)
    cc1_flags = [f"-G{g_value}" if flag.startswith("-G") else flag
                 for flag in CC1_FLAGS] + overrides.get("cc1_flags", [])
    as_flags = [f"-G{g_value}" if flag.startswith("-G") else flag
                for flag in AS_FLAGS]
    if overrides.get("gas_reorder"):
        as_flags.append("-O2")
    if overrides.get("pad_sections"):
        as_flags.remove("-no-pad-sections")
    obj = (output_root if output_root is not None else OUT) / rel.with_suffix(".o")
    obj.parent.mkdir(parents=True, exist_ok=True)
    i_file = obj.with_suffix(".i")
    s_file = obj.with_suffix(".s")

    cpp = [CPP, *CPP_FLAGS]
    if skip_asm:
        cpp.append("-DSKIP_ASM")
    cpp += [f"-I{(ROOT / rel).parent}", src, "-o", i_file]
    r = run(cpp)
    if r.returncode:
        sys.exit(f"[cpp] {rel}\n{r.stderr}")

    r = run([cc1, *cc1_flags, *extra_flags, i_file, "-o", s_file])
    if r.returncode:
        sys.exit(f"[cc1] {rel}\n{r.stdout}{r.stderr}")

    assembly = s_file.read_text()
    for name, flags in overrides.get("function_flags", {}).items():
        if only_function is not None and name != only_function:
            continue
        alternate = obj.with_suffix(f".{name}.s")
        r = run([cc1, *cc1_flags, *extra_flags, *flags, i_file, "-o", alternate])
        if r.returncode:
            sys.exit(f"[cc1] {rel} ({name}: {' '.join(flags)})\n{r.stdout}{r.stderr}")
        assembly = replace_compiler_function(assembly, alternate.read_text(), name)
    s_file.write_text(assembly)

    scheduler_flags = ["--gas-reorder"] if overrides.get("gas_reorder") else []
    r = run([PY, MASPSX, f"--aspsx-version={ASPSX_VERSION}", "--expand-div",
             f"-G{g_value}", *scheduler_flags],
            input=assembly, cwd=ROOT)
    if r.returncode:
        sys.exit(f"[maspsx] {rel}\n{r.stderr}")
    masp_s = obj.with_suffix(".masp.s")
    masp_s.write_text(r.stdout)

    r = run([AS, *as_flags, "-o", obj, masp_s], cwd=ROOT)
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
    byte-matching build) and regenerate objdiff.json's unit list."""
    import json

    # Removed or merged source files must not remain as stale objdiff units.
    src_objs = [OUT / src.relative_to(ROOT).with_suffix(".o")
                for src in sorted((ROOT / "src").rglob("*.c"))]
    if not src_objs:
        sys.exit("no build/src objects; run a full build first")
    if any(not obj.exists() for obj in src_objs):
        sys.exit("missing build/src objects; run a full build first")
    active = {obj.relative_to(OUT) for obj in src_objs}
    for stale in (ROOT / "expected" / "src").rglob("*.o"):
        if stale.relative_to(ROOT / "expected") not in active:
            stale.unlink()
    units = []
    for obj in src_objs:
        rel = obj.relative_to(OUT)
        dst = ROOT / "expected" / rel
        dst.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(obj, dst)
        unit = rel.relative_to("src").with_suffix("").as_posix()
        units.append({
            "name": unit,
            "target_path": f"expected/{rel.as_posix()}",
            "base_path": f"build/{rel.as_posix()}",
            "metadata": {"source_path": f"src/{unit}.c"},
        })
    cfg_path = ROOT / "objdiff.json"
    cfg = json.loads(cfg_path.read_text())
    cfg["units"] = units
    cfg_path.write_text(json.dumps(cfg, indent=2) + "\n")
    print(f"expected/: {len(units)} units snapshotted; objdiff.json updated")


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
