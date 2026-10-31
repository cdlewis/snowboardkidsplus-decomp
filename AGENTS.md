# Agent prerequisites

Before matching, build, or asm-interpretation work in this repo:

1. Read [METHODOLOGY.md](METHODOLOGY.md) in full — the hard rules (100%-only,
   the asm oracle, symbol naming) and the per-function workflow. Do not update methodology unless you're changing the methodology. Status updates are not helpful.
2. The full-image byte gate (`make` → `build/slps_018.23.exe: OK`) must be
   green before AND after your change. Never commit a red gate.
3. `asm/`, `linkers/*.ld`, `linkers/undefined_*` and `expected/` are
   generated — never hand-edit them; change `configs/` and re-run
   `make split` / `make expected` instead.
4. The deep technique catalog (symptom → source-shape fix, permuter usage,
   per-TU flag overrides) lives in the sibling clone
   `../NFSHS-PSX-decomp/METHODOLOGY.md` — same toolchain family; consult it
   when stuck on a near-miss.

## Local matching resources

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
