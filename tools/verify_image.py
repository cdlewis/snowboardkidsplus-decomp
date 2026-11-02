#!/usr/bin/env python3
"""Verify both executable images against the protected, committed checksum."""

from pathlib import Path
from validate_commit import verify_image


if __name__ == "__main__":
    root = Path(__file__).resolve().parent.parent
    verify_image(root)
    print("build/slps_018.23.exe: OK")
