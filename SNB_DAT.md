# SNB.DAT format and loading

This reference describes the `SLPS-01823` disc image used by this repository.
It combines direct inspection of `disc/SNB/SNB.DAT` with matched C implementations
of the game's loading path. Observed bytes and loader behavior are distinguished
from interpretations that still need confirmation.

## File identity and disc context

| Property | Value |
| --- | --- |
| Disc path | `\SNB\SNB.DAT;1` |
| Extracted path | `disc/SNB/SNB.DAT` |
| Size | 18,595,840 bytes (`0x11BC000`) |
| SHA1 | `226b3cd476010a340455b9720b2e37dcd94b7933` |
| Sector size used by the loader | 2,048 bytes (`0x800`) |

The disc also contains the boot executable `SLPS_018.23`, three movie files
under `MV/` (`ATMOV.STR`, `RAMOV.STR`, `OPMOV.STR`), and audio files under `XA/`
(`BGM_S.XA`, `BGM_L.XA`, and `VOICE_01.XA` through `VOICE_08.XA`). These are
separate filesystem files, rather than entries that the loader locates inside
SNB.DAT. `SYSTEM.CNF` supplies the boot configuration. The purpose of the large
`ZERO.NOP` file has not been established here.

[initializeCdFiles](src/main.c) searches for the three movies and SNB.DAT using
`CdSearchFile`. It converts the DAT file's disc position to an integer sector
and stores it in `gDatStartSector`.

## Observed header and likely directory

Direct inspection gives the following layout. Words are little endian.

| File offset | Observed contents | Interpretation |
| --- | --- | --- |
| `0x000` | `u32` value `0x18B` (395) | Likely entry count |
| `0x004`–`0x62F` | 395 `u32` values | Likely DAT-relative byte offsets |
| `0x630`–`0x7FF` | All zero | Padding to the first payload |
| `0x800` onward | Payload bytes | Likely resource entries |

All 395 candidate offsets are strictly increasing, divisible by `0x800`, and
inside the file. The first is `0x800`; the last is `0x11BB800`.

This is strong evidence for a count followed by a sector-aligned offset
directory. The newly decompiled loading functions do **not** parse this
directory, so its complete semantics remain an inference from the bytes and
the callers' read ranges. There are no separate lengths or type fields in
this proposed directory.

An entry's allocated span can provisionally be calculated from the next
offset, using EOF for the final entry. A span may include padding; it is not
necessarily the meaningful payload length.

| Proposed entry, zero based | Byte offset | Allocated span | DAT sector | Sector count |
| --- | --- | --- | --- | --- |
| 0 | `0x800` | `0x2800` | 1 | 5 |
| 1 | `0x3000` | `0x9000` | 6 | 18 |
| 2 | `0xC000` | `0x26800` | 24 | 77 |
| 3 | `0x32800` | `0x16800` | 101 | 45 |
| 4 | `0x49000` | `0xA800` | 146 | 21 |
| 5 | `0x53800` | `0x1000` | 167 | 2 |
| 394 | `0x11BB800` | `0x800` | 9079 | 1 |

The first two spans exactly match the ranges requested by
[queueStartupSectorReads](src/game/unk_8002B3F4.c), independently supporting
this interpretation.

## How the game reads DAT resources

[queueCdSectorCommand and updateCdRequestQueue](src/main.c) establish the
sector-based loading contract:

```text
DAT byte offset = request.sector * 0x800
read byte count = request.sectorCount * 0x800
disc sector    = gDatStartSector + request.sector
destination    = request.destination
```

The queue holds 64 request records. Each record is `0x1C` bytes and includes
a destination pointer, optional filename, sector, sector count, command,
a selector at offset `0x16`, and a processing parameter at offset `0x18`.
`queueCdSectorCommand` sets the selector to nonzero, selecting DAT-relative
sector reads. The dispatcher also supports reading a filesystem filename
through `CdReadFile` when that selector is zero.

Many resource positions and lengths are embedded in executable code or tables.
Examples include `gGridResourceFirstSectorRanges`,
`gGridResourceSecondSectorRanges`, `gMode1SectorRanges`, and
`gOtherModeSectorRanges` in [main_tail.c](src/main_tail.c).
The generic dispatcher receives a range and destination; it does not look up
a resource name or directory index inside DAT.

Reading is asynchronous. The dispatcher advances through nine phases covering
mode setup, a settling delay, seeking, starting and polling reads, and resource
processing. [runGameLoop](src/engine/game_loop.c) services it each frame and
holds tasks marked as waiting for CD completion until the queue becomes idle.

## Processing commands and resource types

The request command selects what happens to a loaded buffer. These values
are executable-side request commands, **not proven on-disc entry type tags**.

| Command | Processing |
| --- | --- |
| 0 | Keep the bytes in RAM without additional processing in this dispatcher |
| 1 | `uploadTimImageWithOffset`: upload a TIM image, with an optional position offset |
| 2 | `uploadTimArchive`: upload multiple TIM images from an offset archive |
| 3–8 | `loadSoundBank`: load sound-bank slots 0–5 respectively |
| 9 | `openMusicSequence`: open a sequence using sound-bank slot 2 |
| 16 | `uploadImage64x256`: upload a 64×256 VRAM rectangle at `(640, 256)` |

The image dimensions above are VRAM transfer dimensions, not a claim about
the logical dimensions or color depth of a displayed texture.

Bit 7 of the command's low byte skips starting a new read. The request still
passes through the read-polling phase, and processing uses `command & 0x7F`.
This supports processing data already in memory.

Command 15 has a separate filename-based streaming path: locate the file,
seek to it, and call `CdRead2(0x1E0)`. It is not an identified DAT resource type.

[loadSoundBank](src/main_remaining.c) passes its header to `SsVabOpenHead`
and its body to `SsVabTransBody`. The dispatcher supplies the request destination
as the header and RAM address `0x80180000` as the body. Mapping the associated
reads is necessary before assigning complete sound-bank boundaries in DAT.
Music-sequence buffers are passed to `SsSeqOpen`.

## Nested archives and known reads

Some payloads contain their own count and offset tables. They should not be
assumed to share every detail of the proposed top-level directory format.

For the TIM archive consumed by [uploadTimArchive](src/main_remaining.c), the
count is read as a `u16` from the first word. Later words contain offsets
relative to the archive base. The implementation clears the offset's low bit
before accessing the image. Whether that bit has a meaning elsewhere remains
unresolved.

The payload at DAT sector 1 starts with words `9, 0x28, 0x330, 0x6F8`.
The payload at sector 6 starts with `22, 0x5C, 0x7BC, 0xF1C`.
Both are consistent with nested count-and-offset layouts; their complete
payload formats and resource identities are not established by these headers
alone.

| Caller | DAT sector | Count | Destination | Observed use |
| --- | --- | --- | --- | --- |
| `queueStartupSectorReads` | 1 | 5 | `0x8017D000` | Raw startup archive |
| `queueStartupSectorReads` | 6 | 18 | `0x801F0000` | Raw startup archive |
| `queueBackgroundTextureSectorCommands` | `0x480` | `0x4A` | `0x80180000` | TIM archive processing, command 2 |
| Same | `0xC9B` | 1 | `0x80180000` | Raw background resource |
| Same | `0xC9C` | 1 | `0x80180800` | Raw background resource |
| Same | `0xC78` | 13 | `0x80181000` | Later uploaded as a TIM image |
| Same | `0xC85` | 4 | `0x80187800` | Later uploaded as a TIM image |

Background callers are in [unk_8002B3F4.c](src/game/unk_8002B3F4.c).
These are examples, not a complete DAT catalogue. A read range can cover
part of an entry or several resources; callers must be compared with directory
boundaries before treating requests as one-to-one entry mappings.

## Executable code and compression: current limits

The examined loading path has no explicit overlay relocation, instruction-cache
flush, or handoff to an entry point in newly loaded memory. The inspected DAT
contains no literal `PS-X EXE` signature. Neither observation excludes
headerless code in a raw resource or an execution path elsewhere in the game.
Executable code inside DAT therefore remains **unproven**.

There is no general decompression stage in `updateCdRequestQueue`.
This does not establish that every resource is uncompressed: individual
consumers may interpret or decode their own formats.

The next useful investigation is to map all sector-read callers onto the
proposed 395 entries, trace the consumers of command-0 buffers, and identify
the formats of the remaining nested archives. A complete catalogue should
record entry spans, actual read ranges, processing commands, RAM destinations,
and evidence for each resource identity.
