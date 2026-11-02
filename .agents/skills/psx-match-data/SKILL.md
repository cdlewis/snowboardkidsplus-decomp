---
name: psx-match-data
description: Move tables, rodata, mutable globals, or BSS into C in Snowboard Kids Plus and verify storage layout.
---

# Match data

## Identify the data owner

Start with a green `./tools/build-and-verify.sh`. Read the generated data assembly
and its consumers. Establish addresses, sizes, section, alignment, and ownership.
PSX data is little endian. Shared objects stay with their owning module and
have declarations in the appropriate header.

## Update the layout and C

Change the data subsegment in `snowboardkidsplus.yaml` to belong to its C unit,
for example from `data` to `.data` with the owning unit's name. Add definitions
in original data order, preserving bytes, relocations, dimensions, padding,
and section placement. Update `symbol_addrs.txt` metadata as needed.

Keep explicit zero initialization, small-data placement, and compiler alignment
in mind. Use section attributes only when the layout requires them. Never
edit generated assembly or linker files.

## Validate

Run `./tools/build-and-verify.sh`, then:

```sh
python3 tools/data_diff.py SYMBOL --size 0xSIZE
python3 tools/data_diff.py --find-first-mismatch
```

The first command checks bytes and address; omit `--size` when symbol metadata
provides it. BSS has no file bytes: check its address, size, alignment, and
references instead. Fix all layout differences and recheck affected consumers.
Refresh verified targets with `./tools/build-and-verify.sh --expected`.
