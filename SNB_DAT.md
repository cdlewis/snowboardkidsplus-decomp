# SNB.DAT format and loading

This reference describes the `SLPS-01823` disc image used by this repository.
It combines direct inspection of `disc/SNB/SNB.DAT` with matched C implementations
of the game's loading path. Observed bytes and loader behavior are distinguished
from interpretations that still need confirmation.

## Extraction, mapping and rebuilding

Run the normal project wrapper:

```sh
./tools/build-and-verify.sh --jobs 4
```

It splits both inputs, builds the executable, maps resource reads, reconstructs
SNB.DAT, and independently verifies both original hashes. The executable still
uses `snowboardkidsplus.yaml`. The DAT uses a companion Splat configuration
generated from its directory and `config/dat_resources.yaml`; both run through
the same wrapper. Do not invoke Splat or Make directly.

Generated outputs are under `build/` and contain original game data; do not
commit them:

| Output | Purpose |
| --- | --- |
| `build/dat/snb_dat.yaml` | Companion Splat configuration: header and 395 binary spans |
| `build/dat/assets/` | Exact header and allocated entry bytes, including padding |
| `build/dat/nested/` | Child spans from consumer-confirmed offset archives |
| `build/dat/catalogue.md` | Entry list and read inventory for inspection |
| `build/dat/catalogue.json` | Full evidence, nested layouts, reads, commands, destinations and RAM views |
| `build/dat/SNB.DAT` | Reconstructed archive, assembled from extracted header and entry files |
| `build/include/game/io/dat_entries.h` | Resource sector/count constants and runtime addresses |
| `build/dat/runtime.ld` | Absolute runtime symbols; allocates no storage |

For read-only inspection without a full build:

```sh
.venv/bin/python tools/snb_dat.py inspect
.venv/bin/python tools/snb_dat.py verify
```

Inspection needs the executable's generated assembly for live fallbacks; run
the wrapper first. `verify` checks current extracted files, nested spans, rebuilt
DAT bytes and catalogue freshness. The wrapper handles `generate`, `map`, and
`pack` in dependency order. Packing reads its output bytes solely from extracted
files and refuses to overwrite the original DAT. It currently preserves the
original layout and requires the original DAT to derive span boundaries; it is
not an asset editor or a disc-image builder.

The tracked manifest assigns stable entry indices, descriptive names where
supported, evidence, known C read tables, and assembly-backed table layouts.
Offsets and allocated lengths come from the protected DAT directory, never a
second hand-maintained list. Unknown identities keep index-based names. C table
fields and calls use generated `DAT_*_SECTOR` and `DAT_*_SECTOR_COUNT` constants;
there is no new runtime resource-lookup table or change to the CD queue ABI.

Runtime addresses remain canonical in `symbol_addrs.txt`. The manifest associates
them with RAM views and evidence; `include/game/io/dat_resources.h` supplies typed
views used by C. The views overlap deliberately across game states. In particular,
the DAT staging region also holds animation/scene data, sound bodies and the movie
ring. Course reads, startup textures, movie buffers and save-record workspaces
reuse parts of the same arena. These are absolute references, not executable
data/BSS definitions or a claim that all views are resident together. Some extent
bounds describe available address space rather than an established allocation.

Fixed-address C views are intentional for matching. Direct linker-object accesses
produce different address-loading instructions from the original immediate
`lui`/`ori` sequences. Generating addresses from the canonical symbol map preserves
the original instructions while removing numeric addresses from consumers.
Archive fields and model/scene/save slots use member access and typed indexing.

### Scene timeline records

The scheduler in [scene_playback.c](src/scene/scene_playback.c) advances a scene
timeline and dispatches image, text, music, CD-audio and sound events. Its loader
is in [scene_loading.c](src/scene/scene_loading.c), and its channel handlers are
in [scene_event_tasks.c](src/scene/scene_event_tasks.c).

`SceneEvent`, declared in `include/game/scene/scene_events.h`, is a `0x18`-byte
record. Its first word is an unsigned scheduled tick. The next halfword selects
`SCENE_EVENT_IMAGE`, `SCENE_EVENT_TEXT`, `SCENE_EVENT_MUSIC`,
`SCENE_EVENT_CD_AUDIO`, or `SCENE_EVENT_SOUND`. Remaining halfwords hold a
resource ID, event-specific mode, duration and parameters; the record also has
previous/next pointers. `parameter14` is deliberately shared: audio uses it for
volume/repeats, while text playback uses it for its reveal interval.

The shared RAM view `DAT_SCENE_WORKSPACE` contains the free/active list heads
and a pool of 295 events. Scene dialogue strings live in separate text blocks;
an event's tick is not a text pointer. Text layout helpers retain their own
declarations in `include/game/text/text_layout.h`. Scene images and event data
are named accordingly in the DAT catalogue. Unidentified fields retain neutral
names, and these semantic names make no claim about original source filenames.

The inventory covers literal C calls, nine C resource tables, and live assembly
calls. Constant propagation through assembly is conservative at branches and
calls, includes delay slots, and retains instruction context. Seven manually
traced table-use descriptions resolve the remaining dynamic assembly reads from
the original executable's halfword tables. A new unknown expression is reported
as unresolved rather than assigned a guessed entry. Table rows describe possible
reads; the report does not claim each row executes in a particular playthrough.

The initial map associates reads with 392 of the 395 entries. Entries 150–152
(sectors `0x9D1`, `0x9D4`, and `0x9E5`) have no identified read in this inventory.
They include a sequence signature, an unclassified payload, and a VAB header
signature respectively; their reachability and relationship remain unproven.
The catalogue distinguishes structurally validated TIMs, signature-based audio
candidates, consumer-confirmed offset archives, and structural archive candidates.
Only confirmed archives receive extracted child files. Child spans may include
padding; their offset words, including low-bit flags, remain recorded verbatim.
The raw top-level spans are authoritative for reconstruction.

Validation checks entry coverage, extracted bytes, nested bounds, read bounds,
known destination extents and catalogue freshness. Dynamic destination expressions
and executable-owned buffer allocations remain recorded for consumer analysis;
the tool does not prove their bounds or resource lifetimes. Unknown compression,
payload formats, VAB body/header pairing outside traced reads and executable code
inside DAT still require investigation.

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

[initializeCdFiles](src/io/cd_queue.c) searches for the three movies and SNB.DAT using
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
[queueStartupSectorReads](src/graphics/background_resources.c), independently supporting
this interpretation.

## How the game reads DAT resources

[queueCdSectorCommand and updateCdRequestQueue](src/io/cd_queue.c) establish the
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
Examples include `gGridResourceFirstSectorRanges` and
`gGridResourceSecondSectorRanges` in [panel_tasks.c](src/menu/panel_tasks.c),
and `gMode1SectorRanges` and `gOtherModeSectorRanges` in
[text_sequence.c](src/text/text_sequence.c).
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

[loadSoundBank](src/audio/sound_manager.c) passes its header to `SsVabOpenHead`
and its body to `SsVabTransBody`. The dispatcher supplies the request destination
as the header and RAM address `0x80180000` as the body. Mapping the associated
reads is necessary before assigning complete sound-bank boundaries in DAT.
Music-sequence buffers are passed to `SsSeqOpen`.

## Nested archives and known reads

Some payloads contain their own count and offset tables. They should not be
assumed to share every detail of the proposed top-level directory format.

For the TIM archive consumed by [uploadTimArchive](src/graphics/texture_upload.c), the
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

Background callers are in [background_resources.c](src/graphics/background_resources.c).
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

The generated catalogue now maps known sector-read callers and tables onto the
395 entries. Further investigation should trace command-0 consumers, establish
the remaining nested formats, and determine whether entries 150–152 are used.
Neither a structural candidate nor a signature alone establishes full resource
identity, compression behavior or executable code.
