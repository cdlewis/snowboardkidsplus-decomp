# PSX graphics macro source map

Paths below are relative to the repository root. Inspect definitions, not just
names. The local SDK's provenance and individual header hashes are recorded in
`lib/psyq-sdk/PROVENANCE.json`. These are the supplied 4.2 reference headers, not
proof of the SDK release or macro spelling used by any particular game unit.

## GPU packets and ordinary vector setters

Primary source: `lib/psyq-sdk/4.2/INCLUDE/LIBGPU.H`.
Project port: `include/psx/libgpu.h` (a subset).

| Operation | Definitions to inspect |
| --- | --- |
| Primitive initialization | `setPolyF3`, `setPolyG3`, `setPolyFT3`, `setPolyGT3`, the corresponding quadrilateral family, `setSprt*`, `setTile*`, `setLine*` |
| Color channels | `setRGB0`, `setRGB1`, `setRGB2`, `setRGB3` |
| Screen coordinates and extents | `setXY0`, `setXY2`, `setXY3`, `setXY4`, `setXYWH`, `setWH`, `setRECT` |
| Texture coordinates | `setUV0`, `setUV3`, `setUV4`, `setUVWH` |
| Primitive flags | `setSemiTrans`, `setShadeTex`; inspect their read-modify-write semantics |
| Packet length, code, and address | `setlen`, `getlen`, `setcode`, `getcode`, `setaddr`, `getaddr` |
| Ordering-table links | `addPrim`, `addPrims`, `catPrim`, `termPrim`, `nextPrim`, `isendprim` |
| Texture page and palette | `getTPage`, `getClut`, `setTPage`, `setClut` |
| Draw state | `setDrawTPage`, `setTexWindow`; compare uppercase draw-state APIs separately |
| Vector fields | `setVector`, `applyVector`, `copyVector`, `addVector` |

Important distinctions:

- An initializer's name does not promise that it initializes every primitive
  byte. Inspect the exact fields it touches, including tag link bits and padding.
- `getTPage`/`getClut` pack values in expressions. `GetTPage`/`GetClut` are
  library APIs; SDK `setTPage`/`setClut` invoke those uppercase APIs. Do not invent
  lowercase replacements inside those setters and call them the SDK versions.
- `nextPrim` follows the encoded DMA link; it is not an allocator or a way to
  advance a contiguous primitive buffer.
- Packed stores, scalar stores, shared-value assignments, and differently
  grouped setters can have different scheduling even when final bytes agree.
  Inspect expansions and compile plausible forms before attributing the source.

## GTE: calls, leaf inlines, and composites

Types and library API declarations: `lib/psyq-sdk/4.2/INCLUDE/LIBGTE.H`.
Project header: `include/psx/libgte.h` includes the SDK's `INLINE_C.H`
directly and adapts the used DMPSX command placeholders to GNU assembler
mnemonics.

| Source header | Purpose and useful search names |
| --- | --- |
| `INLINE_C.H` | C inline GTE access and command macros: `gte_ldv*`, `gte_ldrgb*`, matrix/control-register loaders, `gte_rtps`, `gte_rtpt`, `gte_mvmva`, `gte_nclip`, `gte_avsz*`, lighting/depth-cue commands, `gte_stsxy*`, `gte_strgb*`, `gte_stdp`, `gte_stflg`, depth/result stores |
| `GTEMAC.H` | Composite inline operations built from leaf macros, including `gte_RotTransPers*`, `gte_RotTrans`, `gte_LocalLight`, `gte_LightColor`, and color/lighting combinations |
| `INLINE_O.H` | Alternate C inline implementation with explicit temporary registers and clobbers; inspect rather than assuming equivalence with `INLINE_C.H` |
| `INLINE_A.H` | Assembler-side DMPSX macro definitions; useful for identifying expansion families and hazards, not a C header to include directly |
| `LIBGTE.H` | Vector/matrix/color types, API signatures, constants, and assembly-oriented helper definitions |
| `GTEREG.H`, `GTENOM.H` | Register definitions and assembler GTE command support; consult when identifying COP2 operations and encoded commands |

All SDK names in this table live under `lib/psyq-sdk/4.2/INCLUDE/`.
Composite macros may explain a long instruction sequence that initially looks
handwritten. Conversely, a real call to a geometry library routine should not be
expanded merely because a similarly named inline exists. Check relocations,
calling convention, and actual command/transfer sequence. Preserve supported
inline syntax, operand constraints, memory effects, clobbers, and delay slots.
Do not copy `.word` commands into game code to force a match.

## Higher-level graphics and scratchpad access

- `lib/psyq-sdk/4.2/INCLUDE/LIBGS.H` and `include/psx/libgs.h`: `PACKET`,
  ordering tables, coordinate/object types, `GsOUT_PACKET_P`, sorting and matrix
  APIs, attribute constants, and macros such as `GsIncFrame`, `GsUpdateCoord`,
  `GsSetAzwh`. The reference's `PACKET` is byte-sized; this does not establish
  the type of an arbitrary game buffer variable.
- `lib/psyq-sdk/4.2/INCLUDE/LIBETC.H` and `include/psx/libetc.h`:
  `getScratchAddr`, whose offset is measured in words. Preserve its units.
- `include/psx/scratchpad.h` and `symbol_addrs.txt`: the canonical scratchpad
  declaration and linker identity. A symbol is not an allocation in the image
  and is not interchangeable with a preprocessor constant for code generation.

## Additional reference implementations

The [Psy-Q archive](https://github.com/sozud/psy-q/tree/2266d676f8de8a7e7a96d9cbb92854f0a55a86a1)
is cloned at `../psy-q`; its 3.5/3.6 source directories help resolve version and
implementation questions. Use the function-matching skill's SDK source rules.

Silent Hill provides useful independent comparisons at pinned revision
`cd48ec144ee5d1fdc850042be2775f71d17f3fbc`:

- [SDK graphics headers](https://github.com/shdecompilations/silent-hill-decomp/tree/cd48ec144ee5d1fdc850042be2775f71d17f3fbc/include/psyq).
- [Project GPU helpers](https://github.com/shdecompilations/silent-hill-decomp/blob/cd48ec144ee5d1fdc850042be2775f71d17f3fbc/include/gpu.h): packed coordinate/color/UV setters and combined tag/link helpers.
- [Alternate GTE inlines](https://github.com/shdecompilations/silent-hill-decomp/blob/cd48ec144ee5d1fdc850042be2775f71d17f3fbc/include/inline_no_dmpsx.h).

Keep custom reference-project helpers distinct from standard SDK macros. Their
presence does not prove their use here; inspect implementation and compile
appropriate variants. Do not infer a universal coding style from selected
callers, or assume packed setters can be excluded from store widths alone.

For targeted discovery, use `rg -n` for the macro/API family in the project port,
the corresponding SDK header, and reference callers. Search whole families when
the decompiled output exposes only scalar stores or raw COP2 operations.
