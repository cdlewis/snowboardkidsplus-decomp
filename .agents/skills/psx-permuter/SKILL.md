---
name: psx-permuter
description: Use a decomp-permuter experiment for a structurally correct PSX GCC near match in Snowboard Kids Plus. Applies to source-shape and register-allocation residuals, not missing logic or unknown data layouts.
---

# Prepare and review bounded compiler permutation experiments for PSX

Run commands from the repository root. Paths below are relative to that root.

1. Read root `METHODOLOGY.md`, `DECOMPILATION_LEARNINGS.md`, and the permuter section in the sibling NFSHS technique catalog. Require a full green build. Confirm correct calls, branches, loads/stores, types, and expressions before permutation.
2. Keep the experiment in ignored `nonmatchings/`. The sibling `../snowboardkids-decomp/tools/decomp-permuter/` can be used if installed; it is optional and not a baseline dependency. Read that installation's `README.md` and `USAGE.md` before setting up input. Do not copy its project's IDO/N64 compiler wrapper.
3. A manual workspace needs `base.c`, original `target.o`, executable `compile.sh`, and optional `settings.toml`. The compiler script is called as `./compile.sh input.c -o output.o`. Assemble the original splat function with this project's assembler settings; verify its original text bytes. Retain canonical declarations in the candidate prelude.
4. Adapt the compiler script to the shared Python driver: import `tools/build.py` via its directory on `sys.path`, call `compile_c(input_path, True, output_root=unique_scratch_dir, unit_path=Path("src/OWNING_UNIT.c"), only_function="FUNCTION")`, and copy the returned object to the requested output. Use separate temporary directories per invocation/worker. This preserves per-unit `-G` and function scheduling overrides. Smoke-test it against `tools/match_function.py` before running permutations.
5. Prefer constrained manual alternatives before broad random search. Set a bounded runtime and local worker count explicitly. This workflow needs no distributed workers or uploads. Record compiler, flags, headers, source, and target provenance; rebuild the workspace after any environment change.
6. Treat scores as search hints. Review winners for incorrect operators, side effects, arbitrary casts, or source that bypasses the real headers. Relocation names can differ despite equal linked bytes. Prove a plausible winner using `tools/match_function.py` and then normal integration, owned-data checks, and the full-image gate.

No automatic commit, stop-after-one rule, or fixed attempt count is inherited from the N64 workflows. Follow the user's scope and preserve useful unfinished experiments outside live source.
