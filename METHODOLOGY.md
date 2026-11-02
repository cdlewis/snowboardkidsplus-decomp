# Matching Methodology

Playbook for taking a function to a byte-perfect match, modeled on
[NFSHS-PSX-decomp](../NFSHS-PSX-decomp)'s METHODOLOGY.md (read that for the
full technique catalog — same toolchain family, techniques transfer).

## Hard rules

1. **Target = 100% byte-match only.** A function is done at 100%, not 99%.
   The full-image gate (`./tools/build-and-verify.sh` → sha1 `OK`) must stay green on every commit.
   Preserve promising partial C under `#ifdef NON_MATCHING`, with the original
   `INCLUDE_ASM` in the `#else` branch. Record the best observed instruction
   similarity and word counts beside it; this is a search aid, not a match.
2. **The disassembly oracle is splat's output** (`asm/nonmatchings/<unit>/*.s`
   for assembly-backed functions and `asm/matchings/<unit>/*.s` for matched
   functions; regenerate both with `./tools/build-and-verify.sh`). When in doubt about what a function does,
   read it there — it carries file offset, vram, and raw bytes per line.
3. **When a function matches, name it** — add the real name to
   `configs/symbol_addrs.txt` (`Name = 0xADDR; // type:func`) and re-split so
   asm, C, and the map all agree.
4. **Migrate owned data with the function.** Define its tables and mutable
   globals in the owning C module, carve the corresponding data subsegments
   in the yaml, and verify the original bytes and addresses. Shared globals
   belong to their owning module; consumer declarations remain `extern`.
5. **Keep game code on one compiler.** Do not use a different compiler version
   for a difficult game function. Investigate source shape, flags, and assembler
   behavior instead. The separately linked SDK libraries may use other versions.

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
- Pass the same `-G` setting to cc1, maspsx, and GNU as. External small-data
  declarations (`.extern symbol,size`) affect load-delay scheduling too.
- Function-scoped flags can be tested without splitting a logical module:
  compile the same preprocessed TU with the same compiler and the candidate
  flag, then select the function's complete `.ent`/`.end` region before
  assembling. Preserve the normal data and other functions, check for local
  label collisions, and verify the full image. This is a matching workaround;
  it does not establish the original flags or translation-unit boundaries.
- SDK units may select a different vendored SDK compiler for an individual
  function using the same preprocessed TU and function extraction. Keep game
  code on its single compiler, preserve the normal data and other functions,
  and require the full-image gate. This does not establish original compilers.

### SDK assembler scheduling

Some imported SDK units use GCC 2.7.2 with `gas_reorder` and `pad_sections`
settings in `PER_TU_FLAGS`. That compiler can emit an absolute store macro
followed by a return in reorder mode. Maspsx normally adds a nop after the
return; delegating reorder scheduling to GNU as `-O2` lets the store fill the
slot after address expansion. Maspsx still normalizes pseudo-ops, and explicit
compiler `noreorder` regions retain their slots. This reproduces selected SDK
objects without rewriting instructions; verify each new unit against its oracle.

Object text padding matters too. Enabling assembler section padding may pad a
four-byte `.data` input to sixteen bytes. Keep data in a separately named section
when necessary, map it through yaml's `linker_section`, and verify addresses and
sizes as well as code bytes. Neither setting is enabled globally for game code.

## Per-function workflow

1. Pick an active `INCLUDE_ASM` from a game unit; prefer small leaf functions
   until the compiler version is pinned. `tools/score_functions.py` ranks live
   candidates; `tools/find_similar_functions.py` finds structural references.
2. Draft C: read the asm oracle; `m2c` or decomp.me scratch as a starting
   point. Types/globals go in `include/` headers; symbol names in
   `configs/symbol_addrs.txt`.
3. First test a single-body attempt in ignored `nonmatchings/` with real
   project headers using `tools/match_function.py FUNCTION attempt.c`. It
   inherits the owning unit's settings and checks linked bytes in isolation;
   it requires a fresh verified image and supports extern-only data. Owned
   data and jump tables need full-TU objdiff. See the README for commands.
   Preserve a useful partial attempt in its owning source under `NON_MATCHING`,
   including its best measured similarity. Use `./tools/build-and-verify.sh --objects --non-matching`
   to compile these attempts for objdiff; ordinary builds always use the asm.
   Replace the `INCLUDE_ASM` line with the matching C body, then:
   - `./tools/build-and-verify.sh --objects` + **objdiff** (`objdiff.json` is wired: base =
     the owning `build/src/<unit>.o`, target = `expected/src/<unit>.o`)
     for per-function diff;
   - `./tools/build-and-verify.sh` for the full-image byte gate.
4. Stuck on register-allocation residuals? It's source *shape* — try the
   standard idioms (temp for expr, reordered statements, `do{}while(0)`,
   split/chained assignment) or decomp-permuter. Don't invent flags.
5. After matching: refresh `expected/` **only from a verified build**
   (`./tools/build-and-verify.sh --expected` verifies before snapshotting).

Read [DECOMPILATION_LEARNINGS.md](DECOMPILATION_LEARNINGS.md) for source and
layout techniques adapted from both N64 sibling projects. The repo-local
skills indexed in AGENTS cover function/data matching, renaming, and optional
permuter experiments. After shared-header edits, check every consumer. Check
owned data with `tools/data_diff.py` as well as the full-image gate.

## Reference projects

- **NFSHS-PSX-decomp** (local clone) — the model: build driver, per-TU flag
  overrides, INCLUDE_ASM/maspsx hack, technique catalog, permuter workflow.
- **[Vatuu/silent-hill-decomp](https://github.com/Vatuu/silent-hill-decomp)** —
  canonical PSX matching decomp, same toolchain family.
- **[Xeeynamo/psyz](https://github.com/Xeeynamo/psyz)** — PsyQ SDK library
  decomp; useful once we identify SDK library code in this exe (the memcard/
  libmcrd strings at the top of .rodata say plenty of PsyQ libs are linked in).

## PSY-Q library identification

The game links **PSY-Q 4.2** — identified by signature scan
(`tools/find_psyq.py`, using [psx_psyq_signatures](https://github.com/lab313ru/psx_psyq_signatures)
cloned under `tools/`): 4.2 wins the version ladder by matched bytes
(~184 KB, 407 modules), and the library block is contiguous at
**0x800A8204..0x800D7098** (file 0x98A04..0xC7898), ending exactly at .data.
Everything before it (0x800131B4..0x800A8204) is game code. The scan was a
one-time bootstrap: its module map now lives as the `syslib/*` subsegments in
`snowboardkidsplus.yaml` — **the yaml is the single source of truth for the
file structure**; edit it directly. 636 named functions are wired into
`configs/symbol_addrs.txt` (3 same-body conflicts noted in the scan output).

Implications:
- Game code region is ~600 KB; libs need no decompilation from scratch —
  pull matched C from psyz / sotn-decomp psxsdk lineage when carving them.
- PSY-Q 4.2 ships gcc 2.7.2-era cc1 for user code in most installs — weigh
  toward 2.7.2 when pinning the compiler (verify per-function).

## Status

- Full pipeline proven end-to-end; image rebuilds byte-perfect (all-asm +
  1 trivial C function).
- SDK pinned: PSY-Q 4.2 (sig scan). Game compiler version: unpinned
  (default lane 2.8.1; ladder against 2.7.2 first). sdata/-G: unmapped.
  bss_size: unfilled.
- Matched: 1/2154 split functions (`func_800131B4`, trivial).
- Lib block carved: 377 per-OBJ `c` units (`src/syslib/<lib>/<obj>.c`) +
  15 `asm` gap fillers (`syslib/unk_*` — modules the 4.2 sigs didn't claim;
  identify one → rename/retype it in the yaml). objdiff tracks all 378
  units (list is regenerated by `./tools/build-and-verify.sh --expected`).
