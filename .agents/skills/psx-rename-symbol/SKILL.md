---
name: psx-rename-symbol
description: Rename functions, globals, or source units in Snowboard Kids Plus across symbol definitions, source, and generated comparisons.
---

# Rename a symbol or source unit

## Update symbols and consumers

Start with a green `./tools/build-and-verify.sh`. Names are mapped to addresses
in root `symbol_addrs.txt`; choose a name supported by the function's behavior
or data consumers. Use neutral names for unknown meanings.

```text
firstfileDeviceCallback = 0x800D6010; // type:func
```

Preserve the address and existing type, size, ROM, and definition attributes.
Use `rg` to find and update C definitions, declarations, callers, and live
`INCLUDE_ASM` names. Reuse existing headers rather than adding duplicate externs.

For source-unit renames, also update paths in `snowboardkidsplus.yaml`, source
includes, per-unit build settings, and progress categories. Preserve ownership
and section layout.

## Validate

Run `./tools/build-and-verify.sh --expected` to regenerate assembly, linker
outputs, and comparison metadata from a byte-identical build. Search again for
old names and paths. Never patch generated outputs to conceal a stale reference.
