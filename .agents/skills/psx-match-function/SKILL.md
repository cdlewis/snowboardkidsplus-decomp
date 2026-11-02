---
name: psx-match-function
description: Match an assembly-backed function in Snowboard Kids Plus or diagnose a near match. Use for code matching and shared-header changes.
---

# Match a function

## Prepare an attempt

Run `./tools/build-and-verify.sh` and require a green image. Find the function's
live `INCLUDE_ASM` in `src/` and read its regenerated assembly. Check callers,
existing headers, and related functions for types and behavior.

Create the function's workspace:

```sh
./tools/codex --bootstrap-only FUNCTION
```

Enter the printed directory. It contains `base.c`, the target assembly/object,
preprocessed context, and build/diff helpers. Use real project headers and keep
useful variants in separate files. Run `./build.sh base.c` and `./diff.sh`.
Add `--id ID` when bootstrapping to reuse a workspace without overwriting attempts.

The build helper uses the owning unit's compiler settings. Inspect each attempt's
`result.json`, `diff.txt`, and disassembly. Fix calls, types, offsets, and control flow
before allocation and scheduling. Use meaningful C without inline assembly,
register bindings, dummy side effects, or raw struct-offset accesses.

## Integrate exact C

Replace the fallback when the function matches exactly. Name it in
`symbol_addrs.txt` and update declarations and consumers. Move owned tables and
globals to their C owner with correct sections and placement; shared objects
retain their owner and canonical extern declarations. Data and jump tables
need full-unit comparisons.

For a standalone attempt outside the generated workspace, use
`python3 tools/match_function.py FUNCTION nonmatchings/FUNCTION/attempt.c`.

Useful unfinished C can remain under `NON_MATCHING` with the assembly in the
default branch and its measured similarity and word counts recorded.

## Validate

Run `./tools/build-and-verify.sh --expected --objects`. Verify all consumers
of changed types and declarations. An isolated match is incomplete until the
full executable matches. Keep game code on one compiler; SDK units can have
separate verified settings. Commit only when requested.
