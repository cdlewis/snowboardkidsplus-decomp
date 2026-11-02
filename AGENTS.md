# AGENTS.md

## Repository Overview

This is a matching decompilation of Snowboard Kids Plus (PlayStation,
`SLPS-01823`). The goal is C code that reproduces the original executable
byte for byte.

## Project Structure

- `src/`: matched or partial C, with `INCLUDE_ASM` fallbacks for unmatched code.
- `src/syslib/`: linked PSY-Q SDK units.
- `include/`: shared types and declarations; module-private headers live beside C.
- `asm/nonmatchings/`: generated original assembly for unmatched functions.
- `asm/matchings/`: generated original assembly for matched C functions.
- `symbol_addrs.txt`: canonical function and data names and addresses.
- `snowboardkidsplus.yaml`: executable layout and code/data ownership.
- `nonmatchings/`: ignored scratch attempts and comparison artifacts.
- `expected/`: verified target objects for objdiff; `objdiff.json` maps the units.

`asm/`, `linkers/*.ld`, `linkers/undefined_*`, and `expected/` are generated.
Change source/configuration and regenerate them; never edit them by hand.

## Tools

- `./tools/codex --bootstrap-only FUNCTION`: create a function workspace with
  C, context, target assembly/object, and build/diff helpers. Add `--id ID` to
  reuse a workspace; omit `--bootstrap-only` to open Codex there.
- `./tools/build-and-verify.sh`: clean, regenerate splits, check includes, build,
  independently verify the full image, and generate fresh progress. Extracts
  the disc executable first if needed.
- `./tools/build-and-verify.sh --expected`: also snapshot verified target objects.
- `./tools/build-and-verify.sh --objects [--non-matching]`: also compile C
  comparison objects; the optional flag includes partial implementations.
- `python3 tools/score_functions.py --limit 20`: rank live game candidates;
  add `--sdk` to include SDK functions.
- `python3 tools/find_similar_functions.py FUNCTION`: find structural references.
- `python3 tools/match_function.py FUNCTION nonmatchings/FUNCTION/attempt.c`:
  compile one attempt with the owning unit's settings and compare linked bytes.
- `python3 tools/data_diff.py SYMBOL [--size 0xSIZE]`: check data bytes and address.
- `python3 tools/data_diff.py --find-first-mismatch`: locate an executable mismatch.
- `./tools/install-git-hooks.sh`: install the tracked commit validation hook.

Use the wrapper for every full build, split regeneration, and progress report.
Do not invoke `make`, splat, or `tools/build.py`
directly, or report progress from stale output. Isolated scratch tools and
objdiff's configured object builder are supported; comparison objects do not
prove an executable match. Codex and Claude command hooks enforce this policy;
review and trust the Codex project hook through `/hooks` before relying on it.

Read the relevant repo-local skill when using its workflow:

- [Function matching](.agents/skills/psx-match-function/SKILL.md)
- [Data matching](.agents/skills/psx-match-data/SKILL.md)
- [Symbol renaming](.agents/skills/psx-rename-symbol/SKILL.md)
- [Permuter experiments](.agents/skills/psx-permuter/SKILL.md)

## Tasks

### Decompile assembly to C

When choosing a target rather than working on a requested function, run
`python3 tools/rank_library_functions.py --limit 20` first. Prefer high-similarity
candidates with a clear C reference and no tied alternatives; use complexity
ranking when no suitable library reference remains. Structural similarity does
not establish function identity or a byte match.

#### Step 1: Prepare and match the function

Start with a green `./tools/build-and-verify.sh`. Find the live `INCLUDE_ASM`
and read its regenerated assembly, including addresses and raw bytes. Search
callers and existing declarations to establish argument types and data layouts.
Consult the README's [local SDK references](README.md#local-sdk-references)
before inventing an implementation.

Run `./tools/codex --bootstrap-only FUNCTION` and enter the printed workspace.
Use `./build.sh base.c` and `./diff.sh` to compare attempts. Keep useful variants
in separate files and use real project headers. Inspect each attempt's
`result.json`, `diff.txt`, and disassembly. Owned data and jump tables require
full-unit comparisons. An isolated `EXACT` result must still pass integration.

Fix types, calls, branches, and memory accesses before register allocation.
Try meaningful C source-shape changes, then bounded permuter experiments when
appropriate. Keep game code on one compiler; SDK units may have separate
verified settings. Preserve logical modules when testing local overrides.

#### Step 2: Integrate a successful match

Replace the assembly fallback only with exact C. Give matched functions
names supported by their behavior, and update `symbol_addrs.txt`,
headers, and consumers together. Search existing declarations before adding
new ones; do not make everything `void *` to avoid resolving types.

Migrate owned tables and globals into their C owner with the correct YAML
sections, sizes, addresses, and alignment. Shared globals stay with their
owner and have canonical `extern` declarations. Verify initialized data with
`tools/data_diff.py`; check BSS allocation and addresses separately.

Run `./tools/build-and-verify.sh --expected --objects`. If it fails, the match
is incomplete. Inspect all functions in the changed unit and every consumer of
changed structs or headers, then fix the mismatch. Never use comparison objects
or expected snapshots to hide a failing executable.

Useful unfinished C may remain under `NON_MATCHING`, with the original assembly
in the default branch and the best measured similarity and word counts recorded.

#### Step 3: Finish the requested work

Report the result and verification. Commit only when requested. Before a commit,
stage the complete build-input change and use the tracked validation hook.
Fix a failed hook; never bypass it. Stay within the user's requested scope.

#### When to stop

If a build or experiment tool explicitly instructs you to stop, stop and report
the best score and attempt file, approaches tried, remaining differences, and
whether a different strategy is needed. Preserve useful scratch attempts.
A function is complete only at 100% byte match with a green full-image gate.

## Validation Checklist

Before declaring C changes complete, verify:

- [ ] Field accesses use typed structs and `->` or `.` rather than raw offsets.
- [ ] Parameters and return values have types supported by their consumers.
- [ ] Struct sizes, offsets, signedness, and alignment match the PSX accesses.
- [ ] Every consumer of a changed shared declaration still matches.
- [ ] Owned data has the original bytes, placement, and storage ownership.
- [ ] `./tools/build-and-verify.sh` succeeds before and after the change.

The protected executable SHA1 is `3f1a7abc267ea29bafed1653121613ea467c1ac8`.
Never change expected checksum values in `tools/build.py`,
`snowboardkidsplus.yaml`, or `decomp.yaml`, weaken verification, or bypass hooks
to make a failure pass. The commit hook independently hashes both the disc
executable and rebuilt image against the committed checksum.

### Decompilation Learnings

[DECOMPILATION_LEARNINGS.md](DECOMPILATION_LEARNINGS.md) is a stable technique
reference, not a work log. Do not add function histories, scores, progress, or
commit notes during routine work. Edit it only when the user explicitly requests
changes to the reference.

## Code Quality Standards

Do not write unit tests unless directly instructed by the user.

### Use C and proper SDK macros

Use meaningful C types, expressions, and control flow. Do not force a match with
inline assembly, empty assembly barriers, register bindings, dummy side effects,
or manually encoded instructions. Use the existing PSY-Q graphics macros and
named constants for GPU packets. A passing checksum does not excuse poor C.
Fixed BIOS ABI variables use named pointer macros such as `BIOS_DCB_BASE` and
`BIOS_DCB_BYTES`.

### Avoid raw pointer offsets

For accesses such as `*(type *)((u8 *)ptr + offset)`, identify the containing
struct, array stride, and field offset. Define the layout and use member access
or typed array indexing. Use neutral offset-based names for unknown fields.
Ordinary pointer traversal and fixed hardware/BIOS addresses remain appropriate.

### Struct modification and extension

Search the entire codebase for users before changing a struct. Check nearby
accesses, target-compiler sizes, and all affected functions afterward. PSX is
little endian; verify layouts with the target compiler.

### Avoid redundant declarations

Use `rg` to search `src/` and `include/` for existing prototypes and types.
Reuse canonical headers; remove redundant local declarations when integrating
code. Keep private types in the owning module and shared declarations in headers.

## Code Structure

Use `#pragma once` in project headers. After includes, organize C files as
macros, struct definitions, global variables, function declarations, then
function implementations. Preserve required data
section order and alignment when reorganizing. Preserve existing license notices.
