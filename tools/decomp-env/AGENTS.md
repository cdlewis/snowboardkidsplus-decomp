# Match $functionName

## Workspace

Match `$functionName` from `$unit` against `target.s`. `project/` links to the
repository. `base.c` contains an existing C attempt or initial m2c output when
available. `context.c` contains preprocessed source context for type lookup.
Use the real project headers and the owning unit's compiler settings.

## Build and compare

```sh
./build.sh base.c
./diff.sh
```

Each build keeps its artifacts under `attempts/`, including linked target and
candidate bytes, disassembly, `diff.txt`, and `result.json`. Exit status 0 means
exact bytes, 1 means a mismatch, and 2 means the attempt could not be checked.
Keep useful source variants as `base_1.c`, `base_2.c`, and so on. Fix compiler
errors and inspect each diff before changing the next attempt.

## Matching

Resolve the function's callers, types, field layouts, and purpose first. Use
natural C, typed fields, array indexing, and SDK macros. Do not use assembly,
register bindings, dummy operations, or fake memory layouts to force a match.
A stale-image error means running `./tools/build-and-verify.sh` from `project/`.

## Integration

Once exact, integrate the C in `$unit`, update names and declarations, and
migrate its owned data with the correct layout. Run the full build and byte
gate through `project/tools/build-and-verify.sh --expected --objects`.
A scratch match alone does not complete the function. Commit only when asked.
