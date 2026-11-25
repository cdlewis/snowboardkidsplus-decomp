---
name: psx-match-function
description: Decompile a Snowboard Kids Plus assembly function to byte-matching C or improve an existing partial match. Use for per-function matching, integration, and verification.
---

# Decompile a function to matching C

## Step 1. Match the function in its workspace

From the repository root, start with a green image and prepare the named function:

```sh
./tools/build-and-verify.sh
./tools/codex --bootstrap-only FUNCTION
```

The bootstrap summary identifies the owning source, current inline score, and
compatible archived attempt. Add `--references` for ranked C references or
`--id ID` to resume a workspace. Find the live `INCLUDE_ASM` in `src/`, read
the regenerated target assembly, and check callers and declarations in `src/`
and `include/` for argument types, return values, and field layouts.

Enter the printed workspace and run `./build.sh base.c` for a baseline. Start
each variant in a separate file such as `base_1.c` and run `./build.sh base_1.c`.
Use real project headers and keep each candidate independent of mutable
attempt files. The helper uses the owning unit's compiler settings and archives
source, disassembly, and scores under `attempts/base_N/`.

Use the first differing word and diff excerpt to choose the next change. Fix
calls, types, field accesses, and control flow before register allocation or
instruction scheduling. Use `./diff.sh` for the latest comparison and
`best.json` to locate the highest-scoring archived source. Owned data and jump
tables require a full-unit comparison. For a structurally correct near match,
the [permuter skill](../psx-permuter/SKILL.md) covers bounded experiments.

## Step 2. Integrate and verify an exact match

Return to the repository root. Replace the fallback and its attempt annotation
with the exact C. Give the function a name supported by its behavior and update
`symbol_addrs.txt`, existing declarations, and consumers together. For owned
tables or globals, follow the [data-matching skill](../psx-match-data/SKILL.md)
to verify bytes and placement. Keep shared globals with their owner.

Run `./tools/build-and-verify.sh --expected --objects`. If the executable
differs, inspect every function in the changed unit and each consumer of
changed headers or structs. Fix the mismatch and rerun the wrapper before
reporting an exact match.

## Step 3. Record the result

For a first useful partial match or an improvement, keep the best measured C
under `#ifdef NON_MATCHING` and the original `INCLUDE_ASM` in its `#else`
branch. Put `/* DECOMP_ATTEMPT: inline X/Y */` immediately above the guard,
using the matching and compared word counts. Use `object` for scores that mask
branch and jump destinations. Verify the fallback image and compile the partial
C with `./tools/build-and-verify.sh --expected --objects --non-matching`.

Report the result and verification for the requested function. For Nigel tasks,
commit verified changes and include the hash. For manual work, commit only
when requested. Before committing, run `./tools/install-git-hooks.sh`, stage
the complete build-input change, and fix any hook failure before retrying.

## When to stop

End the attempt loop on exact bytes and proceed to integration. If a build or
experiment tool explicitly says to stop, stop immediately. Preserve the scratch attempts and report the best score and archived source from `best.json`, the approaches tried, the remaining differences, and whether they call for a different strategy. Finish with the requested function.
