"""Shared PSX inspection helpers for matching tools (standard library only).

Adapted workflow ideas: snowboardkids{,2}-decomp's score_functions,
find_similar_functions, and data-differ. Original N64 offsets/byte order and
trained difficulty weights are deliberately not used here.
"""

import re
import struct
from dataclasses import dataclass
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
INCLUDE = re.compile(r'INCLUDE_ASM\s*\(\s*"([^"]+)"\s*,\s*(\w+)\s*\)')
WORD = re.compile(r'/\*\s*([\dA-Fa-f]+)\s+([\dA-Fa-f]+)\s+([\dA-Fa-f]{8})\s*\*/\s*(.*)')
BRANCHES = {"beq", "bne", "beqz", "bnez", "blez", "bgtz", "bltz", "bgez",
            "bltzal", "bgezal", "bc2f", "bc2t"}


def uncomment(source):
    pattern = r'"(?:\\.|[^"\\])*"|\'(?:\\.|[^\'\\])*\'|//[^\n]*|/\*.*?\*/'
    return re.sub(pattern, lambda m: "\n" * m[0].count("\n")
                  if m[0].startswith(("//", "/*")) else m[0], source, flags=re.S)


@dataclass
class AsmFunction:
    name: str
    path: Path
    unit: Path
    address: int
    data: bytes
    instructions: list

    @property
    def opcodes(self):
        return [line.split()[0] for line in self.instructions]

    def metrics(self):
        ops = self.opcodes
        stack = re.search(r'(?:addiu|addu|subu)\s+\$sp,\s*\$sp,\s*(-?0x[\da-fA-F]+|-?\d+)',
                          "\n".join(self.instructions))
        return {"instructions": len(ops), "branches": sum(o in BRANCHES for o in ops),
                "calls": sum(o in {"jal", "jalr"} for o in ops),
                "jumps": ops.count("j"), "unknown_words": ops.count(".word"),
                "stack": abs(int(stack[1], 0)) if stack else 0}


def parse_asm(path, unit=Path(), name=None):
    content = path.read_text()
    label = re.search(r'^(?:glabel|dlabel)\s+(\w+)', content, re.M)
    words = [m for line in content.splitlines() if (m := WORD.search(line))]
    if not words or not label:
        raise ValueError(f"no splat function instructions in {path}")
    address = int(words[0][2], 16)
    if any(int(m[2], 16) != address + i * 4 for i, m in enumerate(words)):
        raise ValueError(f"noncontiguous assembly in {path}")
    return AsmFunction(name or label[1], path, unit, address,
                       b"".join(bytes.fromhex(m[3]) for m in words),
                       [m[4].split("/*", 1)[0].strip() for m in words])


def active_functions(root=ROOT, sdk=False):
    """Use live source includes, not stale files left behind by make split."""
    for source in sorted((root / "src").rglob("*.c")):
        unit = source.relative_to(root)
        if not sdk and unit.parts[:2] == ("src", "syslib"):
            continue
        for match in INCLUDE.finditer(uncomment(source.read_text())):
            path = root / match[1] / (match[2] + ".s")
            if not path.exists():
                raise ValueError(f"missing active assembly {path}; run make split")
            yield parse_asm(path, unit, match[2])


@dataclass
class Symbol:
    name: str
    address: int
    size: int = 0
    kind: int = 0
    section: int = 0


def configured_symbols(root=ROOT):
    result = {}
    for line in (root / "configs/symbol_addrs.txt").read_text().splitlines():
        m = re.match(r'\s*(\w+)\s*=\s*(0x[\da-fA-F]+);(.*)', line)
        if m:
            size = re.search(r'\bsize:(0x[\da-fA-F]+|\d+)\b', m[3])
            result[m[1]] = Symbol(m[1], int(m[2], 16), int(size[1], 0) if size else 0,
                                  2 if "type:func" in m[3] else 1)
    return result


class PsxImage:
    def __init__(self, data):
        self.data = data
        if len(data) < 0x800 or data[:8] != b"PS-X EXE":
            raise ValueError("expected a PS-X EXE with a 0x800-byte header")
        self.address, self.size = struct.unpack_from("<II", data, 0x18)
        if self.size > len(data) - 0x800:
            raise ValueError("truncated PS-X EXE load image")

    def offset(self, address, size=1):
        offset = address - self.address
        if size < 0 or offset < 0 or offset + size > self.size:
            raise ValueError(f"0x{address:08X}+0x{size:X} is not file-backed; check BSS layout separately")
        return offset + 0x800

    def read(self, address, size):
        offset = self.offset(address, size)
        return self.data[offset:offset + size]


class Elf32:
    """Read real ELF symbol sizes/addresses without depending on nm output."""
    def __init__(self, path):
        self.data = path.read_bytes()
        if self.data[:6] != b"\x7fELF\x01\x01" or len(self.data) < 52:
            raise ValueError(f"expected little-endian ELF32: {path}")
        if struct.unpack_from("<H", self.data, 18)[0] != 8:
            raise ValueError(f"expected MIPS ELF: {path}")
        offset = struct.unpack_from("<I", self.data, 32)[0]
        stride, count, names = struct.unpack_from("<HHH", self.data, 46)
        self.sections = [struct.unpack_from("<10I", self.data, offset + i * stride)
                         for i in range(count)]
        strings = self.section_data(names)
        self.section_names = [self.string(strings, s[0]) for s in self.sections]
        self.symbols = {}
        for section in self.sections:
            if section[1] != 2:
                continue
            strings = self.section_data(section[6])
            for pos in range(section[4], section[4] + section[5], section[9]):
                name, value, size, info, _, index = struct.unpack_from("<IIIBBH", self.data, pos)
                name = self.string(strings, name)
                if name and (name not in self.symbols or info >> 4):
                    self.symbols[name] = Symbol(name, value, size, info & 15, index)

    @staticmethod
    def string(data, offset):
        return data[offset:data.index(b"\0", offset)].decode("utf-8", errors="replace")

    def section_data(self, index):
        s = self.sections[index]
        return b"" if s[1] == 8 else self.data[s[4]:s[4] + s[5]]

    def read_symbol(self, name):
        symbol = self.symbols[name]
        if not 0 < symbol.section < len(self.sections):
            raise ValueError(f"{name} has no file-backed ELF section")
        section = self.sections[symbol.section]
        start = symbol.address - section[3]
        data = self.section_data(symbol.section)
        if start < 0 or start + symbol.size > len(data):
            raise ValueError(f"{name} exceeds its file-backed ELF section")
        return data[start:start + symbol.size]


def require_fresh(root=ROOT):
    output = root / "build/slps_018.23.exe"
    elf = root / "build/slps_018.23.elf"
    if not output.exists() or not elf.exists():
        raise ValueError("build artifacts missing; run make first")
    inputs = [root / "snowboardkidsplus.yaml", root / "tools/build.py"]
    for folder, pattern in [("src", "*.c"), ("include", "*.h"),
                            ("configs", "*.txt"), ("tools/maspsx", "*.py")]:
        inputs.extend((root / folder).rglob(pattern))
    oldest = min(output.stat().st_mtime_ns, elf.stat().st_mtime_ns)
    newer = next((p for p in inputs if p.exists() and p.stat().st_mtime_ns > oldest), None)
    if newer:
        raise ValueError(f"build artifacts older than {newer.relative_to(root)}; run make first")


def first_difference(expected, actual):
    return next((i for i, (a, b) in enumerate(zip(expected, actual)) if a != b),
                min(len(expected), len(actual)) if len(expected) != len(actual) else None)
