# Matching Methodology

Playbook for taking a function to a byte-perfect match, modeled on
[NFSHS-PSX-decomp](../NFSHS-PSX-decomp)'s METHODOLOGY.md (read that for the
full technique catalog — same toolchain family, techniques transfer).

## Hard rules

1. **Target = 100% byte-match only.** A function is done at 100%, not 99%.
   The full-image gate (`make` → sha1 `OK`) must stay green on every commit.
2. **The disassembly oracle is splat's output** (`asm/nonmatchings/main/*.s`,
   regenerate with `make split`). When in doubt about what a function does,
   read it there — it carries file offset, vram, and raw bytes per line.
3. **When a function matches, name it** — add the real name to
   `configs/symbol_addrs.txt` (`Name = 0xADDR; // type:func`) and re-split so
   asm, C, and the map all agree.

## Toolchain

- `cpp` (host) → **real GCC PSX `cc1`** (vendored: `tools/gcc/gcc-{2.7.2,2.8.0,2.8.1}-psx`,
  native Linux builds from [decompals/old-gcc](https://github.com/decompals/old-gcc))
  → **[maspsx](https://github.com/mkst/maspsx)** (vendored `tools/maspsx`, ASPSX-2.77
  emulation) → GNU `as` (`mips-linux-gnu`, R3000) → GNU `ld` → objcopy.
- **The game's exact compiler version is NOT yet identified.** Default lane is
  2.8.1 (`SKP_CC1` env overrides). First real matching session should ladder a
  few mid-size functions across 2.7.2 / 2.8.0 / 2.8.1 and pin the winner here.
- `-G` value also unconfirmed (`SKP_G_VALUE`, default 0). The exe has a live
  `$gp` (0x8013AE48) so there IS an sdata zone — mapping it (yaml `sdata`/`sbss`
  subsegments + correct `-G`) is early real work.

## Per-function workflow

1. Pick a function from `src/main.c` (an `INCLUDE_ASM` line); prefer small
   leaf functions until the compiler version is pinned.
2. Draft C: read the asm oracle; `m2c` or decomp.me scratch as a starting
   point. Types/globals go in `include/` headers; symbol names in
   `configs/symbol_addrs.txt`.
3. Replace the `INCLUDE_ASM` line with the C body, then:
   - `make skipasm` + **objdiff** (`objdiff.json` is wired: base =
     `build/src/main.o`, target = `expected/src/main.o`) for per-function diff;
   - `make` for the full-image byte gate.
4. Stuck on register-allocation residuals? It's source *shape* — try the
   standard idioms (temp for expr, reordered statements, `do{}while(0)`,
   split/chained assignment) or decomp-permuter. Don't invent flags.
5. After matching: refresh `expected/` **only from a verified build**
   (`make` prints OK, then `make expected`).

## Reference projects

- **NFSHS-PSX-decomp** (local clone) — the model: build driver, per-TU flag
  overrides, INCLUDE_ASM/maspsx hack, technique catalog, permuter workflow.
- **[Vatuu/silent-hill-decomp](https://github.com/Vatuu/silent-hill-decomp)** —
  canonical PSX matching decomp, same toolchain family.
- **[Xeeynamo/psyz](https://github.com/Xeeynamo/psyz)** — PsyQ SDK library
  decomp; useful once we identify SDK library code in this exe (the memcard/
  libmcrd strings at the top of .rodata say plenty of PsyQ libs are linked in).

## Status

- Full pipeline proven end-to-end; image rebuilds byte-perfect (all-asm +
  1 trivial C function).
- Compiler version: unpinned. sdata/-G: unmapped. bss_size: unfilled.
- Matched: 1/2159 split functions (`func_800131B4`, trivial).
