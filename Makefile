# Thin wrapper over tools/build.py (the canonical driver), NFSHS-PSX-decomp style.
PY ?= .venv/bin/python
SPLAT ?= .venv/bin/splat

.PHONY: all skipasm expected split extract clean hooks report
all:
	$(PY) tools/build.py

skipasm:
	$(PY) tools/build.py --skip-asm

expected:
	$(PY) tools/build.py --update-expected

split:
	$(PY) tools/clean_split_asm.py
	$(SPLAT) split snowboardkidsplus.yaml

extract:
	$(PY) tools/extract_iso.py "Snobow Kids Plus (Japan)/Snobow Kids Plus (Japan).bin" disc

clean:
	$(PY) tools/build.py clean

hooks:
	tools/install-git-hooks.sh

report:
	$(PY) -m mapfile_parser objdiff_report
