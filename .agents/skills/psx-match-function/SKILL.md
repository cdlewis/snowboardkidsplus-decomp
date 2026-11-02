---
name: psx-match-function
description: Match an assembly-backed function in Snowboard Kids Plus, or diagnose a near match. Use this for code matching and shared-header changes; use psx-match-data for standalone data work.
---

# Match a PSX game function using the real compiler and byte gate

Run commands from the repository root. Paths below are relative to that root.

1. Read root `METHODOLOGY.md` and `DECOMPILATION_LEARNINGS.md`; run `./tools/build-and-verify.sh` and require the full-image OK before editing.
2. Find the active include and read its splat assembly. Use `python3 tools/score_functions.py --limit 20` to select small candidates, or `python3 tools/find_similar_functions.py FUNCTION` to find structural references. JP sibling C supplies hypotheses; PSX instructions decide types and behavior.
3. Draft one function in ignored `nonmatchings/FUNCTION/attempt.c`. Include real project headers. Keep canonical prototypes and typed structures; infer layouts from consumers and verify target offsets. Avoid inline assembly, pinned registers, dummy operations, and raw offset casts to force codegen.
4. Run `python3 tools/match_function.py FUNCTION nonmatchings/FUNCTION/attempt.c`. Use `--unit src/path.c` if ownership is ambiguous. Inspect `result.json`, `diff.txt`, and disassembly. The helper inherits real unit/compiler/function flags and leaves live build objects intact. It supports a single body with extern-only data; owned data and jump tables require full-TU objdiff.
5. Fix structural differences first: signedness, argument/return types, loads/stores, control flow, and calls. Then change one source shape at a time. For stubborn allocation residuals consult the NFSHS technique catalog referenced in AGENTS, or psx-permuter.
6. Integrate only exact matches. Name symbols through configs and regenerate splits. Move owned data into C with its function; use psx-match-data for byte/placement checks. Preserve logical modules rather than splitting each function to tune flags.
7. Shared-header edits require rebuilding all consumers. Run `./tools/build-and-verify.sh --objects` to verify the full image and prepare object comparisons. Add `--expected` when refreshing verified targets; the wrapper snapshots them before preparing comparison objects. A scratch EXACT result alone is insufficient.

Keep all game code on one compiler; SDK origin may differ. Document real workflow discoveries in the learnings reference, not attempt histories or progress counters. Commit only when requested.
