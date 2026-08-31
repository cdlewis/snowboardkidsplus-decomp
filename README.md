# Snowboard Kids Plus (Snobow Kids Plus) Decompilation

Matching decompilation of **Snobow Kids Plus (Japan)** for the PlayStation
(`SLPS-01823`), in the [NFSHS-PSX-decomp](https://github.com/Caesar0007/NFSHS-PSX-decomp)
/ [silent-hill-decomp](https://github.com/Vatuu/silent-hill-decomp) mould:
splat-split asm, `INCLUDE_ASM`-based mixed translation units, objdiff
per-function matching, real-compiler (`cc1` + maspsx) build pipeline.

- Boot executable: `SLPS_018.23` (sha1 `3f1a7abc267ea29bafed1653121613ea467c1ac8`)
- Load address: `0x80010000`, ~802 KB of code/data, 2159 split functions

**Read [METHODOLOGY.md](METHODOLOGY.md) before doing matching work.**

## Setup

Supply your own dump of the game as `Snobow Kids Plus (Japan)/` bin+cue
(MODE2/2352). Requires: `mips-linux-gnu` binutils, python3, 32-bit-capable
kernel (the vendored cc1 binaries are static i386).

```sh
python3 -m venv .venv
.venv/bin/pip install pycdlib 'splat64[mips]'

make extract   # bin/cue -> disc/ (iso + SYSTEM.CNF + boot exe)
make split     # splat disassembly -> asm/, src/ skeleton, linker script
make           # full build + byte-verify   -> "build/slps_018.23.exe: OK"
```

## Build lanes

| Command | Purpose |
|---|---|
| `make` | full build (C via cc1+maspsx, asm via GNU as), link, sha1 gate |
| `make skipasm` | C objects only, `-DSKIP_ASM` (objdiff base) |
| `make expected` | snapshot verified objects to `expected/` (objdiff target) |
| `make split` | re-run splat |
| `make clean` | remove `build/` |

## Layout

| Path | Purpose |
|---|---|
| `configs/slps_018.23.yaml` | splat configuration |
| `configs/symbol_addrs.txt` | known symbols (grows as functions are named) |
| `linkers/` | generated linker script + undefined-syms files |
| `asm/nonmatchings/main/` | per-function disassembly (the oracle; regenerated) |
| `src/main.c` | the mixed TU: decompiled C + `INCLUDE_ASM` for the rest |
| `include/` | project headers; `common.h` is every TU's master include |
| `tools/build.py` | canonical build driver (Makefile is a thin wrapper) |
| `tools/gcc/` | vendored PSX gcc 2.7.2 / 2.8.0 / 2.8.1 (decompals/old-gcc) |
| `tools/maspsx/` | vendored maspsx (MIT, from NFSHS-PSX-decomp) |
| `expected/` | objdiff target objects (snapshot of a verified build) |
| `objdiff.json` | objdiff config: base=`make skipasm`, target=`expected/` |

## Notes

- Splat SJIS string decoding is off (`string_encoding: ASCII`) — GNU as would
  re-encode Japanese strings as UTF-8 and break matching.
- Section boundaries are unaligned (`.text` at `0x800131B4`);
  `ld_align_section_vram_end: False` is required.
- Compiler version and `-G` value are not yet pinned — see METHODOLOGY.md.
- Overlay/asset data lives in `SNB/SNB.DAT` on the disc; only the boot exe is
  split so far.
