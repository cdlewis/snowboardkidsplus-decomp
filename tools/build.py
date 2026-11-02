#!/usr/bin/env python3
"""Snowboard Kids Plus (SLPS-01823) matching-decomp build driver.

Pipeline (PSY-Q-faithful):

    src/*.c  --cpp-->  *.i  --cc1 (real gcc 2.x PSX build)-->  *.s
             --maspsx--> GNU as --> ELF .o
    asm/*.s  --GNU as--> ELF .o
    all .o   --GNU ld (linkers/slps_018.23.ld)--> slps_018.23.elf
             --objcopy--> flat PS-EXE, sha1-compared to disc/SLPS_018.23

Usage:
    Internal driver; agents use ./tools/build-and-verify.sh instead.
    python tools/build.py                  # full build + verify
    python tools/build.py --objects        # comparison objects only; no link
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

# Per-TU overrides (flags proven per translation unit go here).
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

SDK_272_FLAGS = {
    "cc1": SDK_CC1, "g_value": "0", "gas_reorder": True,
    "pad_sections": True,
}
SDK_272_GAS_FLAGS = {**SDK_272_FLAGS, "gas_indexed_loads": True}

PER_TU_FLAGS["src/syslib/libcard/card.c"] = SDK_272_FLAGS.copy()
PER_TU_FLAGS["src/syslib/libgte/geo_00.c"] = SDK_272_FLAGS.copy()
PER_TU_FLAGS["src/syslib/libgte/geo_01.c"] = SDK_272_FLAGS.copy()
PER_TU_FLAGS["src/syslib/libgte/sincos.c"] = {"pad_sections": True}
PER_TU_FLAGS["src/syslib/libgte/cor_05.c"] = SDK_272_FLAGS.copy()
PER_TU_FLAGS["src/syslib/libcard/init.c"] = SDK_272_FLAGS.copy()
PER_TU_FLAGS["src/syslib/libc/memmove.c"] = {
    "gas_reorder": True, "pad_sections": True,
}
PER_TU_FLAGS["src/syslib/libetc/vsync.c"] = SDK_272_FLAGS.copy()
PER_TU_FLAGS["src/syslib/libspu/s_sr.c"] = SDK_272_FLAGS.copy()
PER_TU_FLAGS["src/syslib/libsn/read.c"] = SDK_272_FLAGS.copy()
PER_TU_FLAGS["src/syslib/libapi/first.c"] = SDK_272_FLAGS.copy()
PER_TU_FLAGS["src/syslib/libcd/c_011.c"] = {
    "gas_reorder": True, "pad_sections": True,
}
PER_TU_FLAGS["src/syslib/libsnd/midicc.c"] = SDK_272_FLAGS.copy()
for _unit in ("src/syslib/libgs/gs_119.c", "src/syslib/libgs/gs_120.c"):
    PER_TU_FLAGS[_unit] = SDK_272_FLAGS.copy()
for _sdk_unit in (
    "libsnd/vs_mono", "libsnd/vs_vab", "libsnd/vs_vtb", "libsnd/vm_doff", "libsnd/vm_don", "libsnd/vm_key", "libsnd/vm_prog", "libsnd/vm_noff", "libsnd/vm_nowof", "libsnd/vm_seq", "libsnd/vm_n2p", "libsnd/cc_7", "libsnd/cc_11", "libsnd/cc_64", "libsnd/cc_91", "libsnd/cc_98", "libsnd/cc_99", "libsnd/cc_100", "libsnd/cc_101", "libsnd/cc_121", "libsnd/midibend", "libsnd/midiprog", "libsnd/ssinit_c", "libsnd/ut_keyv", "libsnd/ut_gpa", "libgs/gs_001", "libgs/gs_010", "libgs/gs_101", "libgs/gs_121", "libgs/2d_com0", "libspu/s_ini", "libmath/ferr",
    "libspu/s_m_init", "libspu/s_m_util", "libspu/s_gvv", "libspu/s_gvex", "libspu/s_dcb", "libspu/spu",
    "libspu/s_it", "libspu/s_itc", "libspu/s_w", "libspu/s_stm", "libspu/s_n2p", "libspu/s_crwa", "libetc/vmode", "libsnd/sssv", "libsnd/sssattr", "libsnd/vm_stav", "libsnd/playmode", "libsnd/midinote", "libsnd/ut_ako", "libsnd/ssinit", "libsnd/sstable", "libsnd/ut_sva", "libsnd/ut_gva", "libsnd/midimeta", "libsnd/tempo", "libgs/2d_com1", "libgs/2d_sp1",
    "libspu/s_snv", "libspu/s_srv", "libspu/s_stsa", "libsnd/ssstart", "libpress/libpress",
    "libetc/intr", "libetc/intr_vb", "libetc/intr_dma", "libapi/counter",
    "libsnd/ssclose", "libsnd/ssstop", "libsnd/ssplay", "libsnd/sspause", "libsnd/ssreplay", "libsnd/ssvol", "libsnd/sssmv", "libsnd/play", "libsnd/pause", "libsnd/replay", "libsnd/next", "libsnd/ccadsr", "libsnd/de_0", "libsnd/de_1", "libsnd/de_2", "libsnd/de_3", "libsnd/de_4", "libsnd/de_5", "libsnd/de_6", "libsnd/de_7", "libsnd/de_8", "libsnd/de_9", "libsnd/de_10", "libsnd/de_11", "libsnd/de_12", "libsnd/de_13", "libsnd/de_14", "libsnd/de_15", "libsnd/de_16", "libsnd/ut_rev", "libsnd/ut_vvol", "libsnd/vs_vh", "libsnd/vs_vtc",
    "libgpu/ext", "libgpu/sys", "libgpu/p00", "libgpu/p01", "libgpu/p06", "libgpu/p10", "libgpu/p17", "libgpu/p22", "libgpu/p26", "libgpu/p33", "libgpu/p34", "libgs/matrix", "libgs/gs_107", "libgs/gs_110", "libgs/gs_111", "libgs/gs_113", "libgs/gs_122", "libcd/c_002", "libcd/c_003", "libcd/c_004", "libcd/c_005", "libcd/c_007", "libcd/c_008", "libcd/c_010", "libcd/event", "libcd/cdread2", "libcd/cdreade", "libcd/cdrom", "libcd/iso9660", "libds/dscb", "libpad/padseqd",
):
    PER_TU_FLAGS[f"src/syslib/{_sdk_unit}.c"] = SDK_272_FLAGS.copy()

# These complete modules reproduce with one GCC 2.7.2 configuration. GNU as
# expands indexed loads into their destination register when it can do so.
for _unit in (
    "src/syslib/libapi/counter.c", "src/syslib/libsnd/ut_vvol.c",
    "src/syslib/libsnd/cc_7.c", "src/syslib/libsnd/cc_11.c",
    "src/syslib/libsnd/cc_64.c", "src/syslib/libsnd/cc_91.c",
    "src/syslib/libsnd/cc_100.c", "src/syslib/libsnd/cc_101.c",
    "src/syslib/libsnd/cc_121.c", "src/syslib/libsnd/midiprog.c",
    "src/syslib/libsnd/vs_vab.c", "src/syslib/libsnd/vs_vtb.c",
    "src/syslib/libsnd/ut_gpa.c",
):
    PER_TU_FLAGS[_unit] = SDK_272_GAS_FLAGS.copy()

PER_TU_FLAGS["src/syslib/libgpu/sys.c"]["cc1_flags"] = ["-fno-builtin"]
PER_TU_FLAGS["src/syslib/libsnd/vm_seq.c"] = {
    **SDK_272_GAS_FLAGS,
    "function_flags": {
        "_SsVmGetSeqLVol": ["-O1"], "_SsVmGetSeqRVol": ["-O1"],
    },
}
# Both ADSR functions reproduce with this compiler; do not change other SDK units.
PER_TU_FLAGS["src/syslib/libsnd/ccadsr.c"] = {
    "cc1": ROOT / "tools/gcc/gcc-2.8.1-psx/cc1", "g_value": "0",
    "gas_reorder": True, "pad_sections": True,
}
PER_TU_FLAGS["src/syslib/libetc/intr.c"]["function_flags"] = {
    "func_800B5DEC": ["-mno-split-addresses"],
}
PER_TU_FLAGS["src/syslib/libetc/intr.c"]["function_compilers"] = {
    "func_800B5DEC": ROOT / "tools/gcc/gcc-2.8.0-psx/cc1",
}
PER_TU_FLAGS["src/syslib/libgs/gs_001.c"]["function_flags"] = {
    "GsInitGraph2": ["-mno-split-addresses", "-fno-delayed-branch"],
}
PER_TU_FLAGS["src/syslib/libgs/gs_001.c"]["function_compilers"] = {
    "GsInitGraph2": ROOT / "tools/gcc/gcc-2.8.0-psx/cc1",
}
PER_TU_FLAGS["src/syslib/libsnd/cc_0.c"] = {
    "cc1": ROOT / "tools/gcc/gcc-2.8.0-psx/cc1", "g_value": "0",
    "gas_reorder": True, "pad_sections": True,
}
PER_TU_FLAGS["src/syslib/libsnd/cc_10.c"] = {
    "cc1": ROOT / "tools/gcc/gcc-2.8.0-psx/cc1", "g_value": "0",
    "gas_reorder": True, "pad_sections": True,
}
PER_TU_FLAGS["src/syslib/libsnd/ssend.c"] = {
    "cc1": ROOT / "tools/gcc/gcc-2.8.0-psx/cc1", "g_value": "0",
    "gas_reorder": True, "pad_sections": True,
}
PER_TU_FLAGS["src/syslib/libsnd/miditime.c"] = {
    "cc1": ROOT / "tools/gcc/gcc-2.8.0-psx/cc1", "g_value": "0",
    "gas_reorder": True, "pad_sections": True,
}
PER_TU_FLAGS["src/syslib/libgs/gs_103.c"] = {
    "cc1": ROOT / "tools/gcc/gcc-2.8.0-psx/cc1", "g_value": "0",
    "gas_reorder": True, "pad_sections": True,
    "cc1_flags": ["-mno-split-addresses", "-fno-delayed-branch"],
}
PER_TU_FLAGS["src/syslib/libgs/gs_005.c"] = {
    "cc1": ROOT / "tools/gcc/gcc-2.8.0-psx/cc1", "g_value": "0",
    "gas_reorder": True, "pad_sections": True,
}
PER_TU_FLAGS["src/syslib/libspu/spu.c"]["function_flags"] = {"_spu_Fw1ts": []}
PER_TU_FLAGS["src/syslib/libspu/spu.c"]["function_compilers"] = {
    "_spu_Fw1ts": ROOT / "tools/gcc/gcc-2.8.0-psx/cc1",
}
PER_TU_FLAGS["src/syslib/libpad/padportd.c"] = SDK_272_FLAGS.copy()

PER_TU_FLAGS["src/syslib/libspu/s_stsa.c"]["cc1_flags"] = ["-O1"]
PER_TU_FLAGS["src/syslib/libgpu/sys.c"]["function_flags"] = {
    "_cwb": ["-fno-delayed-branch"],
    "DrawSyncCallback": ["-mno-split-addresses"],
    "SetGraphDebug": ["-mno-split-addresses"],
    "gpuGetClipStart": ["-mno-split-addresses"],
    "gpuGetClipEnd": ["-mno-split-addresses"],
    "func_800CD4CC": ["-mno-split-addresses"],
}
PER_TU_FLAGS["src/syslib/libgpu/sys.c"]["function_compilers"] = {
    name: ROOT / "tools/gcc/gcc-2.8.0-psx/cc1"
    for name in ("DrawSyncCallback", "SetGraphDebug", "gpuGetClipStart", "gpuGetClipEnd", "func_800CD4CC")
}

PER_TU_FLAGS["src/syslib/libapi/pad.c"] = {
    "cc1": SDK_CC1, "g_value": "0", "gas_reorder": True,
}

PER_TU_FLAGS["src/syslib/libcd/bios.c"] = {
    "cc1": SDK_CC1, "g_value": "0", "gas_reorder": True,
}

PER_TU_FLAGS["src/syslib/libcd/cdread.c"] = {
    "cc1": SDK_CC1, "g_value": "0", "gas_reorder": True,
    "function_flags": {"func_800B8E68": ["-mno-split-addresses"]},
    "function_compilers": {"func_800B8E68": ROOT / "tools/gcc/gcc-2.8.0-psx/cc1"},
}
PER_TU_FLAGS["src/syslib/libcd/c_009.c"] = {
    "gas_reorder": True, "pad_sections": True,
}
PER_TU_FLAGS["src/syslib/libspu/s_m_f.c"] = {"gas_reorder": True}

# Reproduce this setter's original address calculation and return scheduling.
# This local match does not identify the compiler used for the whole SDK.
PER_TU_FLAGS["src/syslib/libspu/s_snc.c"] = {
    "cc1": ROOT / "tools/gcc/gcc-2.8.1-psx/cc1", "g_value": "0",
    "gas_reorder": True, "pad_sections": True,
    "cc1_flags": ["-mno-split-addresses"],
}
for _unit in ("src/syslib/libsnd/ut_rdel.c", "src/syslib/libsnd/ut_rfb.c"):
    PER_TU_FLAGS[_unit] = {"gas_reorder": True}
PER_TU_FLAGS["src/syslib/libsnd/ut_rdel.c"].update({
    "pad_sections": True,
    "function_flags": {"SsUtSetReverbDelay": ["-mno-split-addresses", "-fno-delayed-branch"]},
})
PER_TU_FLAGS["src/syslib/libsnd/ut_rdep.c"] = {
    "gas_reorder": True, "pad_sections": True,
    "function_flags": {"SsUtSetReverbDepth": ["-mno-split-addresses", "-fno-delayed-branch"]},
}

# Comparison-only flags for guarded partial functions; never affect the image.
NON_MATCHING_FUNCTION_FLAGS = {
    "src/syslib/libgs/gs_001.c": {"func_800BD3FC": ["-mno-split-addresses"]},
    "src/syslib/libc/memmove.c": {"memmove": ["-fno-builtin"]},
    "src/syslib/libspu/s_sk.c": {"SpuSetKey": ["-mno-split-addresses"]},
    "src/syslib/libsnd/vm_key.c": {"_SsVmKeyOff": ["-mno-split-addresses"]},
    "src/syslib/libsnd/vm_aloc2.c": {"_SsVmDoAllocate": ["-mno-split-addresses"]},
    "src/syslib/libsnd/vm_aloc1.c": {"_SsVmAlloc": ["-mno-split-addresses"]},
    "src/syslib/libgs/gs_004.c": {"GsSetOffset": ["-mno-split-addresses"]},
    "src/syslib/libsnd/midiread.c": {"_SsSeqPlay": ["-mno-split-addresses"]},
    "src/syslib/libsnd/ssopenq.c": {"SsSeqOpen": ["-mno-split-addresses"]},
    "src/syslib/libspu/s_m_int.c": {"_spu_gcSPU": ["-mno-split-addresses"]},
    "src/syslib/libsnd/sstick.c": {"SsSetTickMode": ["-mno-split-addresses"]},
    "src/syslib/libgs/gs_123.c": {"Gssub_make_matrix": ["-mno-split-addresses"]},
    "src/syslib/libsnd/vm_pb.c": {"_SsVmPitchBend": ["-mno-split-addresses"]},
    "src/syslib/libpress/build.c": {"DecDCTvlcBuild": ["-mno-split-addresses"]},
    "src/syslib/libetc/intr.c": {"func_800B595C": ["-mno-split-addresses"], "func_800B5A34": ["-mno-split-addresses"]},
    "src/syslib/libsnd/ut_cp.c": {"SsUtChangePitch": ["-mno-split-addresses"]},
    "src/syslib/libsnd/vm_vsu.c": {"_SsVmVSetUp": ["-mno-split-addresses"]},
    "src/syslib/libgs/gs_002.c": {"GsSetDrawBuffOffset": []},
    "src/syslib/libgs/gs_003.c": {"GsSetDrawBuffClip": ["-mno-split-addresses"]},
    "src/syslib/libsnd/vm_n2p.c": {"note2pitch": ["-mno-split-addresses"]},
    "src/syslib/libpad/padcmd.c": {"_padSetMainMode": ["-O2"], "_padLoadActInfo": ["-O2"], "func_800D3180": ["-O2"], "_padRecvAtLoadInfo": ["-O2"], "func_800D2E88": ["-O2"]},
    "src/syslib/libsnd/ssend.c": {"SsEnd": ["-mno-split-addresses"]},
    "src/syslib/libcd/cdread.c": {"CdReadBreak": ["-mno-split-addresses"]},
    "src/syslib/libpad/padseqd.c": {"func_800D44E0": []},
    "src/syslib/libsnd/ssvol.c": {
        name: ["-mno-split-addresses"] for name in ("_SsSndSetVol", "SsSepSetVol")
    },
    "src/syslib/libgpu/sys.c": {
        "ResetGraph": ["-mno-split-addresses"],
        "SetDrawEnv": ["-mno-split-addresses"],
        "func_800CEAD8": ["-mno-split-addresses"],
        "func_800CED08": ["-mno-split-addresses"],
        "MoveImage": ["-mno-split-addresses"],
        "MoveImage2": ["-mno-split-addresses"],
    },
    "src/syslib/libpad/padportd.c": {"func_800D3CF4": []},
    "src/syslib/libgs/gs_005.c": {"GsSetClip": ["-mno-split-addresses"]},
    "src/syslib/libgs/gs_127.c": {
        "GsDefDispBuff2": ["-mno-split-addresses", "-fno-delayed-branch"],
    },
    "src/syslib/libgs/gs_104.c": {
        "GsInit3D": ["-mno-split-addresses", "-fno-delayed-branch"],
    },
    "src/syslib/libcd/c_009.c": {"StGetNext": ["-mno-split-addresses"]},
    "src/syslib/libspu/s_m_f.c": {"SpuFree": ["-mno-split-addresses"]},
    "src/syslib/libetc/intr_dma.c": {
        "setIntrDMA": ["-mno-split-addresses"],
    },
    "src/syslib/libsnd/ssstart.c": {"func_800AC754": ["-O1"], "_SsStart": ["-mno-split-addresses"]},
    "src/syslib/libsnd/ut_rfb.c": {
        "SsUtSetReverbFeedback": ["-mno-split-addresses", "-fno-delayed-branch"],
    },
}

NON_MATCHING_FUNCTION_COMPILERS = {
    "src/syslib/libsnd/ssstart.c": {"_SsStart": ROOT / "tools/gcc/gcc-2.8.0-psx/cc1"},
    "src/syslib/libcd/c_011.c": {"func_800BA430": ROOT / "tools/gcc/gcc-2.8.0-psx/cc1"},
    "src/syslib/libgs/gs_001.c": {"func_800BD3FC": ROOT / "tools/gcc/gcc-2.8.0-psx/cc1"},
    "src/syslib/libcd/sys.c": {name: SDK_CC1 for name in ("CdControl", "CdControlF", "CdControlB")},
    "src/syslib/libcd/cdreade.c": {"CdReadFile": ROOT / "tools/gcc/gcc-2.8.0-psx/cc1"},
    "src/syslib/libc/memmove.c": {"memmove": ROOT / "tools/gcc/gcc-2.8.0-psx/cc1"},
    "src/syslib/libapi/pad.c": {"func_800D6F04": ROOT / "tools/gcc/gcc-2.8.0-psx/cc1"},
    "src/syslib/libds/dscb.c": {name: ROOT / "tools/gcc/gcc-2.8.0-psx/cc1" for name in ("DsSyncCallback", "DsReadyCallback", "DsStartCallback")},
    "src/syslib/libspu/s_sk.c": {"SpuSetKey": ROOT / "tools/gcc/gcc-2.8.0-psx/cc1"},
    "src/syslib/libsnd/vm_key.c": {"_SsVmKeyOff": ROOT / "tools/gcc/gcc-2.8.0-psx/cc1"},
    "src/syslib/libsnd/vm_aloc2.c": {"_SsVmDoAllocate": ROOT / "tools/gcc/gcc-2.8.0-psx/cc1"},
    "src/syslib/libsnd/vm_aloc1.c": {"_SsVmAlloc": ROOT / "tools/gcc/gcc-2.8.0-psx/cc1"},
    "src/syslib/libgs/gs_004.c": {"GsSetOffset": ROOT / "tools/gcc/gcc-2.8.0-psx/cc1"},
    "src/syslib/libsnd/midiread.c": {
        "_SsSeqPlay": ROOT / "tools/gcc/gcc-2.8.0-psx/cc1",
        "_SsSeqGetEof": ROOT / "tools/gcc/gcc-2.8.0-psx/cc1",
    },
    "src/syslib/libsnd/ssopenq.c": {"SsSeqOpen": ROOT / "tools/gcc/gcc-2.8.0-psx/cc1"},
    "src/syslib/libspu/s_m_int.c": {"_spu_gcSPU": ROOT / "tools/gcc/gcc-2.8.0-psx/cc1"},
    "src/syslib/libsnd/sstick.c": {"SsSetTickMode": ROOT / "tools/gcc/gcc-2.8.0-psx/cc1"},
    "src/syslib/libgs/gs_123.c": {"Gssub_make_matrix": ROOT / "tools/gcc/gcc-2.8.0-psx/cc1"},
    "src/syslib/libsnd/vm_pb.c": {"_SsVmPitchBend": ROOT / "tools/gcc/gcc-2.8.0-psx/cc1"},
    "src/syslib/libpress/build.c": {"DecDCTvlcBuild": ROOT / "tools/gcc/gcc-2.8.0-psx/cc1"},
    "src/syslib/libetc/intr.c": {"func_800B595C": ROOT / "tools/gcc/gcc-2.8.0-psx/cc1", "func_800B5A34": ROOT / "tools/gcc/gcc-2.8.0-psx/cc1", "func_800B5C04": SDK_CC1},
    "src/syslib/libsnd/ut_cp.c": {"SsUtChangePitch": ROOT / "tools/gcc/gcc-2.8.0-psx/cc1"},
    "src/syslib/libsnd/vm_vsu.c": {"_SsVmVSetUp": ROOT / "tools/gcc/gcc-2.8.0-psx/cc1"},
    "src/syslib/libpad/padcmd.c": {"_padLoadActInfo": ROOT / "tools/gcc/gcc-2.8.0-psx/cc1", "func_800D3180": ROOT / "tools/gcc/gcc-2.8.0-psx/cc1", "_padRecvAtLoadInfo": ROOT / "tools/gcc/gcc-2.8.0-psx/cc1", "func_800D2E88": ROOT / "tools/gcc/gcc-2.8.0-psx/cc1"},
    "src/syslib/libgs/gs_002.c": {"GsSetDrawBuffOffset": SDK_CC1},
    "src/syslib/libgs/gs_003.c": {"GsSetDrawBuffClip": ROOT / "tools/gcc/gcc-2.8.0-psx/cc1"},
    "src/syslib/libsnd/vm_n2p.c": {
        "note2pitch": ROOT / "tools/gcc/gcc-2.8.0-psx/cc1",
    },
    "src/syslib/libcd/cdread.c": {
        "CdReadBreak": ROOT / "tools/gcc/gcc-2.8.0-psx/cc1",
    },
    "src/syslib/libpad/padseqd.c": {
        "func_800D44E0": ROOT / "tools/gcc/gcc-2.8.0-psx/cc1",
    },
    "src/syslib/libsnd/ssvol.c": {
        name: ROOT / "tools/gcc/gcc-2.8.0-psx/cc1"
        for name in ("_SsSndSetVol", "SsSepSetVol")
    },
    "src/syslib/libgpu/sys.c": {
        "ResetGraph": ROOT / "tools/gcc/gcc-2.8.0-psx/cc1",
        "SetDrawEnv": ROOT / "tools/gcc/gcc-2.8.0-psx/cc1",
        "func_800CEAD8": ROOT / "tools/gcc/gcc-2.8.0-psx/cc1",
        "func_800CED08": ROOT / "tools/gcc/gcc-2.8.0-psx/cc1",
        "MoveImage": ROOT / "tools/gcc/gcc-2.8.0-psx/cc1",
        "MoveImage2": ROOT / "tools/gcc/gcc-2.8.0-psx/cc1",
    },
    "src/syslib/libpad/padportd.c": {
        "func_800D3CF4": ROOT / "tools/gcc/gcc-2.8.0-psx/cc1",
    },
    "src/syslib/libetc/intr_dma.c": {
        "setIntrDMA": ROOT / "tools/gcc/gcc-2.8.0-psx/cc1",
    },
}

PER_TU_FLAGS["src/syslib/libpad/padcmd.c"] = {
    "cc1": SDK_CC1, "g_value": "0", "gas_reorder": True,
    "pad_sections": True, "cc1_flags": ["-O1"],
    "function_flags": {"_padSetActAlign": ["-O2"], "_padSetActAlign_snd": ["-O2"], "_padLoadActInfo_snd": ["-O2"], "_padSendAtLoadInfo": ["-O2"]},
}

PER_TU_FLAGS["src/syslib/libpad/padentry.c"] = SDK_272_FLAGS.copy()

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
PER_TU_FLAGS["src/syslib/libcd/sys.c"]["function_flags"].update({
    name: [] for name in ("CdComstr", "CdIntstr", "CdDataCallback")
})
# SDK-only function compiler selection keeps the rest of this unit on its
# existing compiler; the complete linked image remains the byte oracle.
PER_TU_FLAGS["src/syslib/libcd/sys.c"]["function_compilers"] = {
    name: SDK_CC1 for name in ("CdComstr", "CdIntstr", "CdDataCallback")
}


def run(cmd, **kw):
    return subprocess.run([str(c) for c in cmd], capture_output=True,
                          text=True, **kw)


def replace_compiler_function(normal: str, alternate: str, name: str) -> str:
    """Select one complete .ent/.end region from the same compiler's flag run.

    The original data, declarations, and other functions remain in the normal
    compilation. Rename colliding compiler-local labels in the selected region;
    this changes symbol names only, never instruction content or scheduling.
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
    outside = normal[:start] + normal[end:]
    region = replacement[0].group()
    labels = re.findall(r"^\s*(\$L\d+):", region, re.MULTILINE)
    for label in labels:
        if re.search(rf"^{re.escape(label)}:", outside, re.MULTILINE):
            alternate_outside = alternate[:replacement[0].start()] + alternate[replacement[0].end():]
            if re.search(rf"{re.escape(label)}(?![\w$])", alternate_outside):
                raise ValueError(f"cannot isolate externally referenced label {label} in {name}")
            region = re.sub(rf"{re.escape(label)}(?![\w$])", f"{label}_{name}", region)
    return normal[:start] + region + normal[end:]


def select_function_compiler(unit: Path, name: str, cc1: Path, overrides: dict,
                             non_matching: bool) -> Path:
    selected = overrides.get("function_compilers", {}).get(name, cc1)
    if non_matching:
        selected = NON_MATCHING_FUNCTION_COMPILERS.get(unit.as_posix(), {}).get(name, selected)
    if selected != cc1 and unit.parts[:2] != ("src", "syslib"):
        raise ValueError("function-specific compiler selection is restricted to SDK units")
    return selected


def compile_c(src: Path, comparison: bool, *, output_root: Path = None,
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
    if comparison:
        if os.environ.get("NON_MATCHING") == "1":
            cpp.append("-DNON_MATCHING")
    cpp += [f"-I{(ROOT / rel).parent}", src, "-o", i_file]
    r = run(cpp)
    if r.returncode:
        sys.exit(f"[cpp] {rel}\n{r.stderr}")

    r = run([cc1, *cc1_flags, *extra_flags, i_file, "-o", s_file])
    if r.returncode:
        sys.exit(f"[cc1] {rel}\n{r.stdout}{r.stderr}")

    assembly = s_file.read_text()
    function_flags = dict(overrides.get("function_flags", {}))
    if comparison and os.environ.get("NON_MATCHING") == "1":
        function_flags.update(NON_MATCHING_FUNCTION_FLAGS.get(rel.as_posix(), {}))
    for name, flags in function_flags.items():
        if only_function is not None and name != only_function:
            continue
        alternate = obj.with_suffix(f".{name}.s")
        comparison_cc1 = select_function_compiler(
            rel, name, cc1, overrides, comparison and os.environ.get("NON_MATCHING") == "1")
        r = run([comparison_cc1, *cc1_flags, *extra_flags, *flags, i_file, "-o", alternate])
        if r.returncode:
            sys.exit(f"[cc1] {rel} ({name}: {' '.join(flags)})\n{r.stdout}{r.stderr}")
        assembly = replace_compiler_function(assembly, alternate.read_text(), name)
    s_file.write_text(assembly)

    scheduler_flags = ["--gas-reorder"] if overrides.get("gas_reorder") else []
    if overrides.get("gas_indexed_loads"):
        scheduler_flags.append("--gas-indexed-loads")
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


def is_function_disassembly(path: Path) -> bool:
    return "nonmatchings" in path.parts or "matchings" in path.parts


def main():
    args = sys.argv[1:]
    if "clean" in args:
        shutil.rmtree(OUT, ignore_errors=True)
        return
    comparison = "--objects" in args

    for src in sorted((ROOT / "src").rglob("*.c")):
        compile_c(src, comparison)
        print(f"[cc] {src.relative_to(ROOT)}")
    if comparison:
        return  # objdiff base objects only; no link

    for src in sorted((ROOT / "asm").rglob("*.s")):
        if is_function_disassembly(src):
            continue  # included by C or retained as a matching oracle
        assemble_s(src)
    link_and_verify()

    if "--update-expected" in args:
        update_expected()


if __name__ == "__main__":
    main()
