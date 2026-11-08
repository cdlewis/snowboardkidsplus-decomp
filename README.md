Snowboard Kids Plus  
[![Build](https://github.com/cdlewis/snowboardkidsplus-decomp/actions/workflows/build.yaml/badge.svg?branch=main)](https://github.com/cdlewis/snowboardkidsplus-decomp/actions/workflows/build.yaml?query=branch%3Amain)
[![Code](https://img.shields.io/badge/Code-53.31%25-blue)](#building)
[![Data](https://img.shields.io/badge/Data-60.10%25-blue)](#building)
[![Chat](https://img.shields.io/discord/729856662357278750?color=%237289DA&logo=discord&logoColor=%23FFFFFF)](https://discord.gg/PJzG4F8SpF)
===================

<img src="Snobow_Kids_Plus_Title.jpg" alt="Snobow Kids Plus title screen" width="256">

A matching decompilation of **Snobow Kids Plus (Japan)** for the PlayStation
(`SLPS-01823`).

**This repository does not contain game disc data or extracted assembly. An
existing copy of the game is required. This project is not a port to PC or any
other platform. It extracts the original executable and data from your disc
image and combines reverse-engineered C with the remaining assembly to produce
an identical PlayStation executable.**

## Cloning

Clone this repository:

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
under `tools/gcc/` using these paths:

```text
tools/gcc/gcc-2.8.1-psx/cc1
tools/gcc/gcc-2.8.0-psx/cc1
tools/gcc/gcc-2.7.2-psx/cc1
```

Game C uses GCC 2.8.1; selected SDK units use GCC 2.8.0 or 2.7.2.
The compiler binaries are ignored by Git. maspsx is included in the repository.

## Building

Place your MODE2/2352 disc dump at:

```text
Snobow Kids Plus (Japan)/Snobow Kids Plus (Japan).bin
```

The supported executable is:

| Version | Extracted filename | SHA1 |
| --- | --- | --- |
| Japan (`SLPS-01823`) | `disc/SLPS_018.23` | `3f1a7abc267ea29bafed1653121613ea467c1ac8` |

Build and verify the executable:

```sh
./tools/build-and-verify.sh
```

The output is `build/slps_018.23.exe`. A successful build prints the output path
followed by `OK` and ends with `BUILD SUCCEEDED`, confirming that the executable
matches the original byte for byte.

The wrapper extracts `SLPS_018.23` and `SNB/SNB.DAT` into `disc/` if either is
missing, regenerates assembly, builds the executable, and writes a fresh progress
report to `build/report.json`. Use it after changing sources or configuration.

For parallel or incremental builds:

```sh
./tools/build-and-verify.sh --jobs 8
./tools/build-and-verify.sh --incremental --jobs 8
```

## Reverse-engineering notes

The current build covers the boot executable, `SLPS_018.23`. `SNB/SNB.DAT` is
extracted unchanged; its contents are not yet split or rebuilt. The executable
checksum does not verify a rebuilt DAT file or disc image.

- [AGENTS.md](AGENTS.md) describes the project layout, matching workflow, and
  validation requirements.
- [Function matching](.agents/skills/psx-match-function/SKILL.md) and
  [data matching](.agents/skills/psx-match-data/SKILL.md) cover the matching tools.
- Run `./tools/build-and-verify.sh --expected --objects` to prepare verified
  targets and comparison objects for [objdiff](https://github.com/encounter/objdiff).
  Add `--non-matching` to include partial C in comparison objects.

## Contributing

Contributions are most welcome! There are a variety of ways you can assist:

- Match functions while keeping the full executable byte-identical.
- Investigate and document function names, data structures, and game behavior.
- Identify compiler settings and remaining PSY-Q SDK code.
- Investigate the contents and loading of `SNB/SNB.DAT`.
- Improve setup instructions and test builds on other platforms.

Use the extracted disassembly as the reference for contributions. We cannot
accept contributions based on leaked source code or proprietary knowledge
about the game.

Install the commit validation hook once per clone:

```sh
./tools/install-git-hooks.sh
```

Stage all changed build inputs before committing. The hook runs a clean build
and independently verifies the executable against the protected checksum.

## Acknowledgements

This project builds on the knowledge and tools of the wider decompilation
community. Thanks to the developers of [splat](https://github.com/ethteck/splat),
[maspsx](https://github.com/mkst/maspsx),
[old-gcc](https://github.com/decompals/old-gcc),
[objdiff](https://github.com/encounter/objdiff), and
[psyz](https://github.com/Xeeynamo/psyz), and to contributors to other
PlayStation decompilation projects.
