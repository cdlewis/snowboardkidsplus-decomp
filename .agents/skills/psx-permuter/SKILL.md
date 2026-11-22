---
name: psx-permuter
description: Run a bounded decomp-permuter experiment for a structurally correct PSX GCC near match in Snowboard Kids Plus.
---

# Permute a near match

## Prepare the experiment

Start with a green `./tools/build-and-verify.sh`. Confirm the candidate already
has correct calls, control flow, types, offsets, and expressions. Try localized
source changes before random search.

The project launcher prepares a fresh experiment, smoke-tests its compiler
wrapper, and runs a bounded local search:

```sh
./tools/permuter FUNCTION -s nonmatchings/FUNCTION/attempt.c -j 2 --seconds 300
./tools/permuter https://decomp.me/scratch/SCRATCH_ID --prepare-only
```

Local attempts accept a function name or its generated assembly path.
Pass a decomp.me scratch ID or URL to use its C source and context with the
target bytes and compiler settings from this repository. Use
`--preserve-macros 'gte_.*|setRGB0'` to keep those PSX SDK calls visible to the
permuter.

Use `--prepare-only` to inspect the imported files before searching. Experiments
live under a new `nonmatchings/FUNCTION[-N]/permuter/`; the input is preserved as
`original.c`. The launcher looks for decomp-permuter in `tools/decomp-permuter`,
`../decomp-permuter`, and `../snowboardkids-decomp/tools/decomp-permuter`, in
that order. Override discovery with `--permuter-dir` or `DECOMP_PERMUTER`.
Install its Python dependencies in the project's `.venv` (`toml`, optionally
`Levenshtein` for `--algorithm levenshtein`).
Timeout returns 124 and Ctrl-C returns 130, preserving candidate outputs.
A zero score is not proof of linked bytes matching.

For manual setup, use an installed decomp-permuter, such as `../decomp-permuter/`, and read its
usage instructions. Keep the experiment under `nonmatchings/` with `base.c`,
an original `target.o`, an executable `compile.sh`, and optional settings.
Use real project headers. Verify the target object's original text bytes.

The compile wrapper accepts `input.c -o output.o`. Import `tools/build.py` and
call `compile_c(input_path, True, output_root=unique_scratch_dir,
unit_path=Path("src/OWNING_UNIT.c"), only_function="FUNCTION")`, then copy the
returned object to the requested output. Use a separate temporary directory
per invocation. This preserves the unit's compiler, assembler, and flag settings.

## Run and review

Smoke-test the wrapper against `tools/match_function.py`. Choose a bounded
runtime and local worker count. Recreate the experiment after changing headers,
compiler settings, or target bytes.

Treat scores as search hints. Reject winners that introduce incorrect operators,
arbitrary casts, side effects, or fake layouts. Relocation names may differ
while linked bytes agree.

## Validate

Check a plausible winner with `tools/match_function.py`, integrate it, and run
`./tools/build-and-verify.sh --expected --objects`. Verify owned data and shared
header consumers as needed. Commit only when requested.
