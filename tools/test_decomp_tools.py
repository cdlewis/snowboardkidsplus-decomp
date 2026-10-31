"""Behavior checks for PSX diagnostics; fixtures contain no game data."""

import os
import struct
import tempfile
import unittest
from pathlib import Path

from data_diff import compare_symbol
from decomp import (Elf32, PsxImage, Symbol, active_functions,
                    first_difference, require_fresh, uncomment)
from find_similar_functions import normalize_instruction


def image(payload, address=0x80010000):
    header = bytearray(0x800)
    header[:8] = b"PS-X EXE"
    struct.pack_into("<II", header, 0x18, address, len(payload))
    return PsxImage(bytes(header) + payload)


def elf_fixture(path):
    names = b"\0.shstrtab\0.strtab\0.symtab\0.text\0"
    strings = b"\0probe\0"
    code = bytes.fromhex("0800e00300000000")
    symbols = bytes(16) + struct.pack("<IIIBBH", 1, 0x80010000, len(code), 0x12, 0, 4)
    chunks = [names, strings, symbols, code]
    data = bytearray(52)
    offsets = []
    for chunk in chunks:
        offsets.append(len(data))
        data.extend(chunk)
    table = len(data)
    data.extend(bytes(40))
    for i, (name, kind, address, link, stride) in enumerate([
        (".shstrtab", 3, 0, 0, 0), (".strtab", 3, 0, 0, 0),
        (".symtab", 2, 0, 2, 16), (".text", 1, 0x80010000, 0, 0),
    ]):
        data.extend(struct.pack("<10I", names.index(name.encode()), kind, 0, address,
                                offsets[i], len(chunks[i]), link, 0, 4, stride))
    data[:16] = b"\x7fELF\x01\x01\x01" + bytes(9)
    struct.pack_into("<HHIIIIIHHHHHH", data, 16, 2, 8, 1, 0, 0, table, 0,
                     52, 0, 0, 40, 5, 1)
    path.write_bytes(data)
    return code


class ImageTests(unittest.TestCase):
    def test_header_mapping_and_little_endian_value(self):
        target = image(bytes.fromhex("78563412"))
        self.assertEqual(target.offset(0x80010002), 0x802)
        self.assertEqual(struct.unpack("<I", target.read(0x80010000, 4))[0], 0x12345678)

    def test_missing_tail_is_a_mismatch(self):
        self.assertEqual(first_difference(b"abc\0\0", b"abc"), 3)
        self.assertEqual(first_difference(b"abc", b"abx"), 2)
        self.assertIsNone(first_difference(b"abc", b"abc"))

    def test_bss_and_out_of_range_are_not_byte_matched(self):
        target = image(b"abcd")
        for address, size in [(0x80010004, 2), (0x8000FFFF, 1), (0x80010000, -1)]:
            with self.assertRaises(ValueError):
                target.read(address, size)

    def test_invalid_and_truncated_executables_fail(self):
        for data in [b"not an exe", image(b"abcd").data[:-1]]:
            with self.assertRaises(ValueError):
                PsxImage(data)

    def test_equal_bytes_at_wrong_address_still_fail(self):
        target = image(b"abcdabcd")
        original = {"flag": Symbol("flag", 0x80010000, 4)}
        linked = {"flag": Symbol("flag", 0x80010004, 4)}
        self.assertFalse(compare_symbol("flag", target, target, linked, original)[0])

    def test_equal_bytes_with_wrong_symbol_size_fail(self):
        target = image(b"abcdabcd")
        original = {"flag": Symbol("flag", 0x80010000, 8)}
        linked = {"flag": Symbol("flag", 0x80010000, 4)}
        self.assertFalse(compare_symbol("flag", target, target, linked, original)[0])

    def test_changed_data_and_exact_data(self):
        symbols = {"flag": Symbol("flag", 0x80010000, 4)}
        target = image(b"abcd")
        self.assertTrue(compare_symbol("flag", target, target, symbols, symbols)[0])
        self.assertFalse(compare_symbol("flag", target, image(b"abxd"), symbols, symbols)[0])

    def test_elf_symbol_address_size_and_section_bytes(self):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "fixture.elf"
            code = elf_fixture(path)
            elf = Elf32(path)
            self.assertEqual(elf.symbols["probe"].address, 0x80010000)
            self.assertEqual(elf.symbols["probe"].size, 8)
            self.assertEqual(elf.read_symbol("probe"), code)


class DiscoveryTests(unittest.TestCase):
    def test_only_live_includes_are_ranked(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            (root / "src").mkdir()
            (root / "asm/nonmatchings/unit").mkdir(parents=True)
            include = 'INCLUDE_ASM("asm/nonmatchings/unit", live);'
            (root / "src/unit.c").write_text('/* ' + include.replace('live', 'old') + ' */\n' + include)
            assembly = ('glabel live\n/* 800 80010000 0800E003 */ jr $ra\n'
                        '/* 804 80010004 00000000 */ nop\nendlabel live\n')
            (root / "asm/nonmatchings/unit/live.s").write_text(assembly)
            (root / "asm/nonmatchings/unit/old.s").write_text(assembly.replace('live', 'old'))
            functions = list(active_functions(root))
            self.assertEqual([f.name for f in functions], ["live"])
            self.assertEqual(functions[0].data.hex(), "0800e00300000000")

    def test_comments_are_removed_without_changing_string_paths(self):
        self.assertEqual(uncomment('"asm/path//name" /* hidden */ // more\n'), '"asm/path//name"  \n')

    def test_numeric_and_abi_registers_normalize_together(self):
        self.assertEqual(normalize_instruction("addu $2,$4,$8"),
                         normalize_instruction("addu $v0,$a0,$t0"))
        self.assertEqual(normalize_instruction("lw $2,4($28)"), "lw $vN,4($gp)")
        self.assertNotEqual(normalize_instruction("lw $2,4($sp)"),
                            normalize_instruction("lw $2,8($sp)"))

    def test_stale_build_is_rejected(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            (root / "build").mkdir()
            (root / "src").mkdir()
            for name in ["slps_018.23.exe", "slps_018.23.elf"]:
                path = root / "build" / name
                path.touch()
                os.utime(path, ns=(100, 100))
            source = root / "src/unit.c"
            source.touch()
            os.utime(source, ns=(200, 200))
            with self.assertRaisesRegex(ValueError, "older than"):
                require_fresh(root)


if __name__ == "__main__":
    unittest.main()
