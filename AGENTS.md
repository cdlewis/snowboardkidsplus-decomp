# Agent prerequisites

Before matching, build, or asm-interpretation work in this repo:

1. Read [METHODOLOGY.md](METHODOLOGY.md) in full — the hard rules (100%-only,
   the asm oracle, symbol naming) and the per-function workflow.
2. The full-image byte gate (`make` → `build/slps_018.23.exe: OK`) must be
   green before AND after your change. Never commit a red gate.
3. `asm/`, `linkers/*.ld`, `linkers/undefined_*` and `expected/` are
   generated — never hand-edit them; change `configs/` and re-run
   `make split` / `make expected` instead.
4. The deep technique catalog (symptom → source-shape fix, permuter usage,
   per-TU flag overrides) lives in the sibling clone
   `../NFSHS-PSX-decomp/METHODOLOGY.md` — same toolchain family; consult it
   when stuck on a near-miss.
