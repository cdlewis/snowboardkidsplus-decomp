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
under `tools/gcc/`. The current build requires this path:

```text
tools/gcc/gcc-2.8.1-psx/cc1
```

GCC 2.7.2 and 2.8.0 are optional matching candidates at
`tools/gcc/gcc-2.7.2-psx/cc1` and `tools/gcc/gcc-2.8.0-psx/cc1`.
The compiler binaries are ignored by Git;
maspsx is included in the repository.

## Building

Place your MODE2/2352 disc dump at:

```text
Snobow Kids Plus (Japan)/Snobow Kids Plus (Japan).bin
```

Extract the boot executable, generate the disassembly, then build and verify:

```sh
make extract
make split
make
```

The supported executable is:

| Version | Extracted filename | SHA1 |
| --- | --- | --- |
| Japan (`SLPS-01823`) | `disc/SLPS_018.23` | `3f1a7abc267ea29bafed1653121613ea467c1ac8` |

A successful build prints:

```text
build/slps_018.23.exe: OK
```

This confirms that the rebuilt executable's SHA1 matches the original. Run
`make` for subsequent builds; run `make split` again after changing the splat
configuration or symbol names.

Additional build commands:

| Command | Purpose |
| --- | --- |
| `make skipasm` | Compile C objects with `-DSKIP_ASM` for objdiff comparison |
| `make expected` | Run a full verified build, snapshot target objects to `expected/`, and update `objdiff.json` |
| `make clean` | Remove `build/` |

To generate a progress report after a successful build, activate the virtual
environment and run:

```sh
source .venv/bin/activate
python3 -m mapfile_parser objdiff_report
```

`decomp.yaml` supplies the map and assembly paths and writes the report to
`build/report.json`. The command prints overall, game, and PSY-Q SDK code
progress. Assembly functions remain unmatched even though the executable
rebuilds byte-perfectly; data progress is excluded because it has no matching
markers. Re-run `make` after source changes to refresh the map before reporting.
Detection uses assembly files on disk, so stale nonmatching `.s` files can
cause matched functions to remain counted as unmatched.

## Reverse-engineering notes

Read [METHODOLOGY.md](METHODOLOGY.md) before doing matching work. It documents
the matching rules, compiler selection, per-function workflow, and current
status. The full-image byte verification must stay green; functions are
considered matched only at 100%.

- The game-wide compiler version and `-G` value are not yet pinned. The five
  matched RNG functions shared with the N64 Japanese game have verified local
  `-G8` and function-scoped scheduling flags in `tools/build.py`; all game C
  uses the same compiler. Run `python3 tools/test_maspsx.py` and
  `python3 tools/test_build.py` to check the assembler and flag-selection logic.
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
| `tools/build.py` | Build driver used by the Makefile |
| `objdiff.json` | Per-function comparison configuration |

`asm/`, linker scripts and undefined-symbol files in `linkers/`, and `expected/`
are generated. Update the configuration and regenerate them with `make split`
or `make expected` rather than editing them by hand. The per-function
disassembly in `asm/nonmatchings/` is the reference for matching.

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
[Snowboard Kids decompilation](https://github.com/cdlewis/snowboardkids-decomp).
Thanks also to the developers of [splat](https://github.com/ethteck/splat),
[maspsx](https://github.com/mkst/maspsx),
[old-gcc](https://github.com/decompals/old-gcc),
[objdiff](https://github.com/encounter/objdiff), and
[psyz](https://github.com/Xeeynamo/psyz).
