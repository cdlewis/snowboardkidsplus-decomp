#!/usr/bin/env python3
"""Install checksummed native compilers from decompals/old-gcc release 0.17.

The cdk build is cygnus-2.7.2-970404 SN32.3.7.0004, used by Psy-Q 4.1 CCPSX.
The psx builds retain the independently verified SDK compiler configurations.
"""

import argparse
import hashlib
import io
import os
from pathlib import Path
import tarfile
import tempfile
import time
from urllib.error import HTTPError, URLError
from urllib.request import Request, urlopen


ROOT = Path(__file__).resolve().parent.parent
BASE_URL = "https://github.com/decompals/old-gcc/releases/download/0.17"
ARCHIVES = {
    "gcc-2.7.2-cdk": "42bb0df96db11a9b5d2e23d78bdc962791f40046280d3d360da93fe5eef6f0bb",
    "gcc-2.7.2-psx": "500a459b3485e885a8d302cac23c2a4632f3900e03a09153f6190699fd723571",
    "gcc-2.8.0-psx": "1a3c956fe8aea5ebdb251749d95de2c84f023530584d7bd663744b5ec24050b7",
    "gcc-2.8.1-psx": "f6f6e883942d4d3289d048236c672e71ed410e546aaae8ff655952f1567e1be0",
}


def install(name):
    request = Request(f"{BASE_URL}/{name}.tar.gz", headers={"User-Agent": "skp-toolchain"})
    print(f"Fetching {name} from old-gcc release 0.17...", flush=True)
    for attempt in range(4):
        try:
            with urlopen(request, timeout=60) as response:
                data = response.read()
            break
        except (URLError, TimeoutError) as error:
            if (attempt == 3 or
                    isinstance(error, HTTPError) and error.code not in (429, 500, 502, 503, 504)):
                raise
            print(f"Retrying {name}: {error}", flush=True)
            time.sleep(2 ** attempt)
    if hashlib.sha256(data).hexdigest() != ARCHIVES[name]:
        raise ValueError(f"{name}: compiler archive checksum mismatch")
    with tarfile.open(fileobj=io.BytesIO(data), mode="r:gz") as archive:
        member = archive.getmember("cc1")
        if not member.isfile():
            raise ValueError(f"{name}: cc1 must be a regular file")
        compiler = archive.extractfile(member).read()
    destination = ROOT / "tools/gcc" / name
    destination.mkdir(parents=True, exist_ok=True)
    with tempfile.NamedTemporaryFile(dir=destination, delete=False) as output:
        temporary = Path(output.name)
        try:
            output.write(compiler)
            output.close()
            temporary.chmod(0o755)
            os.replace(temporary, destination / "cc1")
        finally:
            temporary.unlink(missing_ok=True)
    print(f"Installed {name}/cc1 (verified release 0.17)")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("compilers", nargs="*", help="compiler names; defaults to all required builds")
    args = parser.parse_args()
    names = args.compilers or list(ARCHIVES)
    for name in names:
        if name not in ARCHIVES:
            parser.error(f"unknown compiler {name}; choose from {', '.join(ARCHIVES)}")
    for name in names:
        install(name)


if __name__ == "__main__":
    main()
