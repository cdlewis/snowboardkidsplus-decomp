#!/usr/bin/env python3
"""Convert MODE2/2352 bin to ISO, then read SYSTEM.CNF and extract the boot exe and SNB.DAT."""
import io
import pathlib
import sys

import pycdlib

BIN = pathlib.Path(sys.argv[1])
OUT = pathlib.Path(sys.argv[2])
OUT.mkdir(parents=True, exist_ok=True)

iso_path = OUT / "disc.iso"
if not iso_path.exists():
    with BIN.open("rb") as f, iso_path.open("wb") as o:
        while True:
            sector = f.read(2352)
            if len(sector) < 2352:
                break
            # sync(12) + header(4) + subheader(8), then 2048 user data (Mode2 Form1)
            o.write(sector[24 : 24 + 2048])

iso = pycdlib.PyCdlib()
iso.open(str(iso_path))
for child in iso.list_children(iso_path="/"):
    if child.file_identifier() not in (b".", b".."):
        print(child.file_identifier().decode())

buf = io.BytesIO()
iso.get_file_from_iso_fp(buf, iso_path="/SYSTEM.CNF;1")
cnf = buf.getvalue().decode()
print("---- SYSTEM.CNF ----")
print(cnf)

boot = [l for l in cnf.splitlines() if l.upper().startswith("BOOT")][0]
exe_name = boot.split("=")[1].strip().replace("cdrom:", "").lstrip("\\/").split(";")[0]
print("exe:", exe_name)
with (OUT / exe_name.replace("\\", "_").replace("/", "_")).open("wb") as f:
    iso.get_file_from_iso_fp(f, iso_path="/" + exe_name + ";1")
(OUT / "SNB").mkdir(parents=True, exist_ok=True)
with (OUT / "SNB/SNB.DAT").open("wb") as f:
    iso.get_file_from_iso_fp(f, iso_path="/SNB/SNB.DAT;1")
iso.close()
