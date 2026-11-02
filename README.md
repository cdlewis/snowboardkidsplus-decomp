# Snowboard Kids Plus

<img src="Snobow_Kids_Plus_Title.jpg" alt="Snobow Kids Plus title screen" width="256">

A matching decompilation of **Snobow Kids Plus (Japan)** for the PlayStation
(`SLPS-01823`), the PlayStation version of
[Snowboard Kids](https://github.com/cdlewis/snowboardkids-decomp).

**An existing copy of the game is required. This repository does not include
game disc data or extracted assembly. This project is not a port to PC
or any other platform: it combines reverse-engineered C with assembly and data
extracted from your copy to produce an identical PlayStation executable.**

The current build covers the boot executable, `SLPS_018.23`. Overlay and asset
data in `SNB/SNB.DAT` is not yet split or rebuilt.

## Cloning

```sh
git clone https://github.com/cdlewis/snowboardkidsplus-decomp.git
cd snowboardkidsplus-decomp
```

## Dependencies

The current build is verified on Linux (x86-64). The GCC binaries require a
kernel capable of running static 32-bit x86 executables.

System packages:

- make
- git
- python3 and venv/pip
- cpp (host C preprocessor)
- binutils-mips-linux-gnu

On Debian/Ubuntu:

```sh
sudo apt install make git python3 python3-venv python3-pip cpp binutils-mips-linux-gnu
```

Install Python dependencies in a virtual environment:

```sh
python3 -m venv .venv
.venv/bin/pip install pycdlib 'splat64[mips]' 'mapfile-parser>=2.13.2,<3'
```

Install the native Linux PSX GCC binaries from
[decompals/old-gcc](https://github.com/decompals/old-gcc/releases/tag/0.17)
under `tools/gcc/`. The current build requires these paths:

```text
tools/gcc/gcc-2.8.1-psx/cc1
tools/gcc/gcc-2.7.2-psx/cc1
```

Game C uses GCC 2.8.1; selected SDK units use GCC 2.7.2.
GCC 2.8.0 is an optional matching candidate at `tools/gcc/gcc-2.8.0-psx/cc1`.
The compiler binaries are ignored by Git;
maspsx is included in the repository.

## Building

Place your MODE2/2352 disc dump at:

```text
Snobow Kids Plus (Japan)/Snobow Kids Plus (Japan).bin
```

Build and verify through the same entry point used by the sibling Snowboard
Kids projects:

```sh
./tools/build-and-verify.sh
```

The wrapper extracts the boot executable if missing, cleans the build,
regenerates split assembly, checks every source `INCLUDE_ASM`, builds the full
executable, and independently verifies both the original and rebuilt images
against the committed checksum. It then
writes a fresh progress report to `build/report.json`. Any failed step fails
the command.

The supported executable is:

| Version | Extracted filename | SHA1 |
| --- | --- | --- |
| Japan (`SLPS-01823`) | `disc/SLPS_018.23` | `3f1a7abc267ea29bafed1653121613ea467c1ac8` |

A successful build prints `build/slps_018.23.exe: OK` and ends with
`BUILD SUCCEEDED`. Run the wrapper again after changing sources, symbols,
headers, or split configuration.

| Command | Purpose |
| --- | --- |
| `./tools/build-and-verify.sh` | Clean, regenerate, verify the executable, and report progress |
| `./tools/build-and-verify.sh --expected` | Also snapshot verified targets to `expected/` and update `objdiff.json` |
| `./tools/build-and-verify.sh --objects` | Also prepare `SKIP_ASM` C objects for objdiff after verification |
| `./tools/build-and-verify.sh --objects --non-matching` | Include guarded partial C in the comparison objects |

`--expected` and `--objects` can be combined; targets are snapshotted before
comparison objects are compiled. Comparison builds do not establish new matches.

Agents must use the wrapper instead of direct make, splat, build-driver, or
progress-report commands. The Makefile remains an internal build interface.
Codex and Claude command hooks enforce this policy. Codex users must review
and trust the project hook using `/hooks` before it runs; see the
[official hook documentation](https://learn.chatgpt.com/docs/hooks).

Progress counts assembly-backed functions as unmatched even when the executable
rebuilds exactly. Data progress is excluded because it has no matching markers.
Each wrapper run clears and regenerates the function disassemblies:
`asm/nonmatchings/` contains assembly fallbacks, and `asm/matchings/` retains
original instructions for matched C functions. Regenerating these files before
reporting prevents stale assembly from undercounting matched C.

## Reverse-engineering notes

Read [METHODOLOGY.md](METHODOLOGY.md) before doing matching work. It documents
the matching rules, compiler selection, and per-function workflow. The full-image byte verification must stay green; functions are
considered matched only at 100%.

- The game-wide compiler version and `-G` value are not yet pinned. The five
  matched RNG functions shared with the N64 Japanese game have verified local
  `-G8` and function-scoped scheduling flags in `tools/build.py`; all game C
  uses the same compiler.
- The linked SDK has been identified as PSY-Q 4.2. Its library units are split
  under `src/syslib/`.
- Splat uses `string_encoding: ASCII` to preserve Japanese string bytes, and
  `ld_align_section_vram_end: False` to preserve unaligned section boundaries.

Key project files:

| Path | Purpose |
| --- | --- |
| `snowboardkidsplus.yaml` | Splat configuration and executable layout |
| `configs/symbol_addrs.txt` | Known function and data symbols |
| `src/main.c`, `src/main_remaining.c` | Mixed C/assembly game units |
| `src/engine/random.c` | Five matched RNG functions, table, and indices |
| `src/engine/game_task_scheduler.c` | Shared frame-counter storage; scheduler code remains in assembly |
| `src/syslib/` | PSY-Q library units |
| `include/` | Project headers; `common.h` is the master include |
| `tools/build-and-verify.sh` | Supported build, verification, and progress entry point |
| `tools/build.py` | Internal compiler/linker driver used by the Makefile |
| `objdiff.json` | Per-function comparison configuration |

`asm/`, linker scripts and undefined-symbol files in `linkers/`, and `expected/`
are generated. Update the configuration and regenerate them with `./tools/build-and-verify.sh`
or `./tools/build-and-verify.sh --expected` rather than editing them by hand. The per-function
disassembly in `asm/nonmatchings/` is the reference for matching.

## SDK source imports

The linked PSY-Q 4.2 libraries have reusable C in the public
[PsyZ decompilation](https://github.com/Xeeynamo/psyz/tree/main/decomp), which
targets PSY-Q 4.0. It is incomplete, and versions differ, so imports must still
match this executable exactly. Its native porting implementation is a separate
reference and is not a substitute for matching SDK source.

Imported functions cover CD wrappers and BCD position conversion, mono/stereo
selection, sound startup, damper control, and SPU transfer state/mode. The main
MDEC decoder module is entirely C, including its tables, register pointers, and
diagnostic strings. Video-mode accessors are
also reconstructed, including their owned four-byte state. See
[THIRD_PARTY.md](THIRD_PARTY.md) for source provenance and the preserved license.
Shared VM/SPU state remains with the existing data objects; consumers use
canonical `extern` declarations. The libc BIOS trampolines and many GTE routines
were assembly, so replacing them with C implementations would change the code.

Selected SDK units have verified local GCC 2.7.2 settings in `tools/build.py`.
Their reorder-mode assembly uses maspsx normalization plus GNU as `-O2` delay
slot scheduling and original object padding. These are scoped settings, not
proof of the compiler for every SDK object. The game compiler is unchanged.

## Matching tools

[DECOMPILATION_LEARNINGS.md](DECOMPILATION_LEARNINGS.md) collects techniques
adapted from both N64 sibling repos, with the PSX differences called out.
[AGENTS.md](AGENTS.md) indexes four repo-local skills for function matching,
data matching, symbol renaming, and optional permuter experiments.

After `./tools/build-and-verify.sh`, rank live game functions and find structural references:

```sh
python3 tools/score_functions.py --limit 20
python3 tools/find_similar_functions.py func_8008C670 --top 10
# Include an explicit sibling reference tree if available:
python3 tools/find_similar_functions.py func_8008C670 --asm-dir ../snowboardkids-decomp/asm/nonmatchings
```

SDK functions are excluded by default; add `--sdk` to include them. Ranking
uses a transparent complexity heuristic; similarity compares normalized MIPS
instructions. Neither score proves a match. Discovery follows active source
includes, so stale local assembly files do not become candidates.

Keep a single-function attempt in ignored `nonmatchings/`, including real
project headers and declaring referenced data `extern`. With a fresh green
build, compile and compare it without changing live build/expected objects:

```sh
./tools/build-and-verify.sh
python3 tools/match_function.py FUNCTION nonmatchings/FUNCTION/attempt.c
```

The helper inherits the owning unit's compiler and flags; use
`--unit src/path.c` if ownership is ambiguous. It writes linked byte checks,
disassembly, a diff, and provenance in `result.json` under the scratch folder.
`EXACT` requires equal bytes, size, and address. Integrate the C and run the
normal full build afterward. Owned data and jump tables require full-TU
objdiff instead of this single-body helper.

Verify a data symbol's address and bytes, or locate a full-image difference:

```sh
python3 tools/data_diff.py gRandomTable
python3 tools/data_diff.py SYMBOL --size 0x100
python3 tools/data_diff.py --find-first-mismatch
```

Data checks derive PSX load addresses from the executable header and read
little-endian ELF symbols. They reject stale builds and non-file-backed BSS.
An explicit size can fill missing symbol metadata. Comparisons return 0 for
an exact match, 1 for a mismatch, and 2 for unsupported or missing input.
`./tools/build-and-verify.sh` remains the full-image gate.

These tools use only the Python standard library and the existing compiler
pipeline; the N64 difficulty model and its Python dependencies are not needed.

Partial implementations use `#ifdef NON_MATCHING` with the original assembly
in the default branch. Their comments record the best observed instruction
similarity from `tools/match_function.py`: its ordered matching four-byte words
divided by the larger instruction-stream length, including any padding in the
target extent. This is a reproducible search aid, not objdiff's percentage or
proof of correctness. Only a byte-exact comparison and the full executable gate
establish a match. Normal and expected-object builds do not enable the define.
Comparison-only settings may select a different SDK compiler for a partial
function; they do not change the compiler or bytes used by the normal build.
## Commit validation

Install the tracked Git hook once per clone:

```sh
./tools/install-git-hooks.sh
```

The hook rejects changes to the expected SHA1 in the build script and YAML
metadata and runs the same clean build-and-verify workflow, including fresh
splits, live include checks, independent image verification, and progress reporting.
A commit proceeds only when the image matches
`disc/SLPS_018.23` byte for byte. It also independently hashes both files
against the committed checksum. It does not refresh `expected/` or stage files.

Stage all changed build inputs before committing. The hook rejects unstaged or
untracked files under `src/`, `include/`, `configs/`, and `tools/`, plus unstaged
build configuration, so validation covers what will actually be committed.
Unrelated notes and local files such as `.envrc` can remain unstaged. Generated
assembly, linker outputs, and build/expected objects must remain unstaged.
If `core.hooksPath` already points elsewhere, the installer leaves it intact;
chain `tools/git-hooks/pre-commit` from that hook setup.

## Contributing

Contributions are welcome! Useful areas include:

- Match functions and keep the full executable byte-identical.
- Investigate and document function names, data structures, and game behavior.
- Identify the original compiler settings and remaining SDK code.
- Document disc assets and overlays beyond the boot executable.
- Improve setup instructions and test builds on other platforms.

Use the extracted disassembly as the reference for contributions. Contributions
must not be based on leaked source code or proprietary knowledge about the game.

## Acknowledgements

This project builds on the tools and knowledge of the decompilation community,
including [NFSHS-PSX-decomp](https://github.com/Caesar0007/NFSHS-PSX-decomp),
[silent-hill-decomp](https://github.com/Vatuu/silent-hill-decomp), and the sibling
[Snowboard Kids decompilation](https://github.com/cdlewis/snowboardkids-decomp)
and [Snowboard Kids 2 decompilation](https://github.com/cdlewis/snowboardkids2-decomp).
Thanks also to the developers of [splat](https://github.com/ethteck/splat),
[maspsx](https://github.com/mkst/maspsx),
[old-gcc](https://github.com/decompals/old-gcc),
[objdiff](https://github.com/encounter/objdiff), and
[psyz](https://github.com/Xeeynamo/psyz).
