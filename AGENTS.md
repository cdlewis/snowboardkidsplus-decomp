# Agent prerequisites

Before matching, build, or asm-interpretation work in this repo:

1. Read [METHODOLOGY.md](METHODOLOGY.md) in full — the hard rules (100%-only,
   the asm oracle, symbol naming) and the per-function workflow. Do not update methodology unless you're changing the methodology. Status updates are not helpful.
2. The full-image byte gate (`./tools/build-and-verify.sh` → `build/slps_018.23.exe: OK`) must be
   green before AND after your change. Never commit a red gate.
3. `asm/`, `linkers/*.ld`, `linkers/undefined_*` and `expected/` are
   generated — never hand-edit them; change `configs/` and re-run
   `./tools/build-and-verify.sh` / `./tools/build-and-verify.sh --expected` instead.
4. The deep technique catalog (symptom → source-shape fix, permuter usage,
   per-TU flag overrides) lives in the sibling clone
   `../NFSHS-PSX-decomp/METHODOLOGY.md` — same toolchain family; consult it
   when stuck on a near-miss.

## Local matching resources

Use `./tools/build-and-verify.sh` for every full build, split regeneration,
and progress report. It cleans the build, regenerates function disassemblies,
checks live includes, builds and independently verifies the image,
then generates fresh progress. Missing disc extraction is performed first.
Agents must not invoke `make`, splat, `tools/clean_split_asm.py`, or
`tools/build.py` directly, or run the progress reporter on stale output.
Use `--expected` to snapshot verified targets and `--objects` (optionally
`--non-matching`) to prepare comparison objects after the full gate.
Isolated scratch tools and objdiff's configured object builder remain supported.
Never treat comparison objects as proof of an executable match.

Codex and Claude PreToolUse hooks enforce the command policy. For Codex,
review and trust the project hook in `/hooks`; new or changed hooks do not
run until trusted. These hooks guide agent commands; the tracked Git hook
independently enforces validation when committing.

Read [DECOMPILATION_LEARNINGS.md](DECOMPILATION_LEARNINGS.md) for techniques
adapted from the two N64 sibling projects, including PSX-specific limits.
Use real project headers in experiments; verify every consumer after shared
type changes. Structural similarity and permuter scores are search aids, never
proof of a match.

Repo-local skills (read the relevant `SKILL.md` when applying its workflow):

- [psx-match-function](.agents/skills/psx-match-function/SKILL.md): select,
  isolate, match, and integrate game functions.
- [psx-match-data](.agents/skills/psx-match-data/SKILL.md): move owned data to C
  and verify bytes, addresses, and storage layout.
- [psx-rename-symbol](.agents/skills/psx-rename-symbol/SKILL.md): rename symbols
  or units through config and regenerated outputs.
- [psx-permuter](.agents/skills/psx-permuter/SKILL.md): prepare and review
  bounded source-shape experiments with the real PSX compiler pipeline.

See the README's matching tools section for candidate ranking, structural
search, isolated compilation, data diffs, and tool checks. Scratch attempts
belong in ignored `nonmatchings/`; never use them to replace live build or
expected objects. These tools do not change the full-image gate requirement.

For SDK matching, consult the README's [local SDK references](README.md#local-sdk-references)
for sibling checkout paths, source coverage, toolchain clues, and reuse limits.

## Commit validation

Install the tracked validation hook with `./tools/install-git-hooks.sh`. Never bypass a failed
hook: fix its reported input, build, or checksum failure. Stage the complete
build-input change before committing; unrelated local files can stay unstaged.
The hook regenerates splits, checks live includes, and requires the
full executable to match the original bytes. It does not stage generated files
or update expected objects.

The protected executable SHA1 is `3f1a7abc267ea29bafed1653121613ea467c1ac8`.
Agents must not change the expected checksums in `tools/build.py`,
`snowboardkidsplus.yaml`, or `decomp.yaml`, weaken validation, or bypass the
hook to make a failing build pass. The hook rejects checksum-value changes
and independently hashes both the disc executable and rebuilt image against
the committed checksum. This value was verified against a fresh read from
the original MODE2 disc BIN and the initial project history.
