# Thin wrapper over tools/build.py (the canonical driver), NFSHS-PSX-decomp style.
PY ?= .venv/bin/python
SPLAT ?= .venv/bin/splat

.PHONY: all skipasm expected split extract clean check
all:
	$(PY) tools/build.py

skipasm:
	$(PY) tools/build.py --skip-asm

expected:
	$(PY) tools/build.py --update-expected

split:
	$(SPLAT) split snowboardkidsplus.yaml

extract:
	$(PY) tools/extract_iso.py "Snobow Kids Plus (Japan)/Snobow Kids Plus (Japan).bin" disc

clean:
	$(PY) tools/build.py clean

check:
	$(PY) -m unittest discover -s tools -p 'test_*.py'
