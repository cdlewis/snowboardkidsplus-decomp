---
name: psx-match-data
description: Migrate tables, rodata, mutable globals, or BSS declarations into C in Snowboard Kids Plus. Use for data ownership, section placement, alignment, and symbol byte comparisons.
---

# Match PSX initialized data and verify storage ownership and layout

Run commands from the repository root. Paths below are relative to that root.

1. Read root `METHODOLOGY.md` and the data section of `DECOMPILATION_LEARNINGS.md`. Start with a full green `./tools/build-and-verify.sh`.
2. Establish original addresses, sizes, consumers, and owner from config, splat, and code. Distinguish private tables from shared objects. Put shared definitions in their owning unit and declarations in canonical headers.
3. Decode PSX data as little endian. Do not reuse N64 big-endian subword offsets. Preserve actual string bytes, terminators, padding, table dimensions, and relocations rather than guessing from display text.
4. Define C data in original order. Inspect object sections, map addresses, sizes, and alignment. Explicit zero initialization and `-G` affect `.data`, `.sdata`, `.bss`, and `.sbss`; small-data externs can also change instruction scheduling. Use section attributes only when supported by placement evidence.
5. Change yaml/config ownership and symbol metadata. Never hand-edit generated asm, linker files, undefined-symbol files, or expected objects. Confirm no duplicate definitions or holes.
6. Run `./tools/build-and-verify.sh`, then `python3 tools/data_diff.py SYMBOL` (or `--size 0xSIZE` if size metadata is unavailable). It checks the original address as well as bytes. Use `python3 tools/data_diff.py --find-first-mismatch` to locate whole-image differences, including header and EOF changes. Freshness errors mean rebuild first.
7. BSS is not file-backed and cannot be byte-matched against the disc. Check target address, allocation size, alignment, boundaries, and all references separately. Do not report a byte MATCH for BSS.
8. Recheck affected code and require the full-image gate. Update expected objects with `./tools/build-and-verify.sh --expected` after verification.
