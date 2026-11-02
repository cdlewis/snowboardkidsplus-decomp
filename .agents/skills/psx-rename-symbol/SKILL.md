---
name: psx-rename-symbol
description: Assign evidence-backed names to functions, globals, or source units in Snowboard Kids Plus. Use when a rename affects config symbols, includes, unit paths, linker references, or comparison metadata.
---

# Rename a PSX symbol or unit consistently through generated outputs

Run commands from the repository root. Paths below are relative to that root.

1. Read root `METHODOLOGY.md`; begin with green `./tools/build-and-verify.sh`. Establish the name from behavior and consumers. Use neutral names for unknown bits instead of inventing semantics.
2. Search with `rg` across `src/`, `include/`, `configs/`, yaml, and build settings. Make canonical symbol changes in `configs/symbol_addrs.txt`; preserve addresses, sizes, `type:func`, and definition attributes. Update C names and declarations together.
3. For a unit rename, update yaml subsegment paths, source paths, per-TU flags in `tools/build.py`, progress categories in `decomp.yaml`, and any documentation references. Preserve layout and module ownership.
4. Update live INCLUDE_ASM paths with the source/config rename. Never patch generated linker aliases, assembly, undefined files, or expected objects to hide a stale reference.
5. Run `./tools/build-and-verify.sh --expected` to regenerate function disassemblies, require byte-identical output, and refresh objdiff unit metadata from the verified build. Search again for obsolete source/config references; inspect the diff for unrelated changes.
