---
name: psx-remove-addressing-defines
description: Remove per-file absolute and GP addressing switches in Snowboard Kids Plus while preserving the original PSX instruction bytes.
---

# Remove addressing defines

Start with a verified build and work one compilation unit at a time. Keep its verified compiler and assembler settings.

1. Trace the defines through the headers in include order. Identify which declarations they change and which symbols the unit actually references. In the original assembly, check for absolute `%hi` and `%lo` relocations and for `%gp_rel` references. Check the C types and array bounds before changing a declaration.
2. Remove the unit's addressing defines. Use the shared declaration where it already produces the required access. For each remaining absolute reference that needs an explicit declaration, add a typed `extern` with `__attribute__((section(".data")))` in the consuming C file before its uses. Keep the canonical symbol and existing storage definition. With this repo's GCC and `tools/cc1psx.py`, the attribute suppresses the assembler's small-data hint for that external symbol.
3. Search for remaining users of each removed switch. Collapse the shared header conditional when its last user is gone. If a switch also renames a symbol or changes its declaration type, account for that behavior before deleting it.
4. Compare the unit's instructions, relocations, and any owned data with the verified baseline. Run `./tools/build-and-verify.sh --expected --objects` before accepting the change. If bytes differ, inspect the first changed instruction or section and refine the declaration in that unit. Check the full image again before applying the pattern elsewhere.
