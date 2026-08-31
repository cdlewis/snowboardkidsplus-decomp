# Thin wrapper over tools/build.py (the canonical driver), NFSHS-PSX-decomp style.
PY ?= .venv/bin/python
SPLAT ?= .venv/bin/splat

.PHONY: all skipasm expected split extract clean
all:
	$(PY) tools/build.py

skipasm:
	$(PY) tools/build.py --skip-asm

expected:
	$(PY) tools/build.py --update-expected

split:
	cd configs && ../$(SPLAT) split slps_018.23.yaml

extract:
	$(PY) tools/extract_iso.py "Snobow Kids Plus (Japan)/Snobow Kids Plus (Japan).bin" disc

clean:
	$(PY) tools/build.py clean
