"""Update README Shields.io badges after build-and-verify generates fresh progress."""

import json
from pathlib import Path
import re

from mapfile_parser import MapFile

ROOT = Path(__file__).resolve().parent.parent


def data_progress():
    # report_data is disabled: without data markers its percentage is a false
    # 100%. Instead count initialized data by linked storage owner. As with the
    # code report, named SDK library objects count as recovered; split asm does not.
    matched = total = 0
    for segment in MapFile.newFromMapFile(ROOT / "build/slps_018.23.map"):
        if segment.name == ".header":
            continue
        for section in segment:
            if section.isFill or section.sectionType.removeprefix(".").split(".")[0] not in {
                "rodata", "data", "sdata",
            }:
                continue
            total += section.size
            owner = section.filepath.as_posix()
            if owner.startswith(("build/src/", "build/sdk/psyq42/")):
                matched += section.size
    if not total:
        raise ValueError("No initialized data found in the verified map")
    return matched, total


def main():
    report = json.loads((ROOT / "build/report.json").read_text())
    measures = report["measures"]
    code = 100 * int(measures["matched_code"]) / int(measures["total_code"])
    matched_data, total_data = data_progress()
    data = 100 * matched_data / total_data
    badges = "\n".join(
        f"[![{label}](https://img.shields.io/badge/{label}-{percent:.2f}%25-blue)]"
        "(#building)"
        for label, percent in (("Code", code), ("Data", data))
    )
    readme = ROOT / "README.md"
    text = readme.read_text()
    pattern = r"<!-- progress-badges:start -->.*?<!-- progress-badges:end -->"
    replacement = (
        "<!-- progress-badges:start -->\n" + badges + "\n<!-- progress-badges:end -->"
    )
    updated, count = re.subn(pattern, lambda _: replacement, text, flags=re.DOTALL)
    if count != 1:
        raise ValueError("README must contain exactly one progress-badges block")
    if updated != text:
        readme.write_text(updated)
    print(f"README badges: Code {code:.2f}%, Data {data:.2f}%")


if __name__ == "__main__":
    main()
