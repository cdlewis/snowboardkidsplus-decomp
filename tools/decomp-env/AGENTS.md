# Match $functionName

## Workspace

Match `$functionName` from `$unit` against `target.s`. `project/` links to the
repository. `base.c` contains an existing C attempt or initial m2c output when
available. `context.c` contains preprocessed source context for type lookup.
Use the real project headers and the owning unit's compiler settings.

## Build and compare

```sh
./build.sh base.c
```

Make the minimum changes needed to compile `base.c` before measuring a baseline.
Each build receives the next unused label `base_1`, `base_2`, and so on. The
helper copies unnumbered input to that numbered C file without overwriting an
existing variant. Create subsequent changes in new `base_N.c` files; preserve
earlier variants. Keep attempts self-contained with real project headers; never
include another mutable candidate C file.

Each build archives its source under `attempts/base_N/base_N.c`, alongside linked
target and candidate bytes, disassembly, `diff.txt`, and `result.json`. Failed
builds also retain their source and receive an unscored log entry. Exit status
0 means exact bytes, 1 means a mismatch, and 2 means the attempt could not be
checked. Review the score change, first differing word, and diff excerpt before
making the next source change. Use `./diff.sh` when you need the full latest diff.
Full build diagnostics are available in the attempt's `compile.log`.

`match_log.txt` records each attempt's score, status, archived source path, and
source hash. Each build prints the best attempt; `best.json` identifies its
archived source and artifacts. Ties retain the earliest attempt, with exact
bytes taking priority over rounded similarity. `latest.json` and `./diff.sh`
refer to the latest successful comparison, which may differ from the best.
Use the best attempt's archived `diff.txt` when reviewing the best result.

Stop on exact bytes, an explicit build-tool stop instruction, or 20 attempts
without improvement. Report the best score and archived source, approaches
tried, remaining differences, and whether a different strategy is needed.

## Matching

Resolve the function's callers, types, field layouts, and purpose first. Use
natural C, typed fields, array indexing, and SDK macros. Do not use assembly,
register bindings, dummy operations, or fake memory layouts to force a match.
Read the relevant sections of `project/DECOMPILATION_LEARNINGS.md` and record
function-specific findings and useful observations in workspace `LEARNINGS.md`.
Keep the repository technique reference free of function histories and scores.
Change one meaningful aspect at a time. If several attempts reach the same
plateau, preserve the best and reconsider the function's behavior and simplest
typed C structure before trying more compiler-shaping changes.
If you are having trouble matching the stack-frame size, consider using an
unused local padding array, such as `u8 padding[0x20];`. Its original purpose
does not need to be known. Verify the resulting frame size with the target
compiler.
A stale-image error means running `./tools/build-and-verify.sh` from `project/`.

## Integration

Once exact, integrate the C in `$unit`, update names and declarations, and
migrate its owned data with the correct layout. Run the full build and byte
gate through `project/tools/build-and-verify.sh --expected --objects`.
A scratch match alone does not complete the function. Commit only when asked.
