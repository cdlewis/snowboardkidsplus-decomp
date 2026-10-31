"""Regression checks for small-data hazards and SDK assembler scheduling."""

import sys
import shutil
import subprocess
import tempfile
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent / "maspsx"))
from maspsx import MaspsxProcessor


class ExternalSmallDataTests(unittest.TestCase):
    def process(self, limit=8, size=2):
        # The load/store pair is the dependency in randomNextMain. A $gp
        # store has no address-expansion instruction to hide the load delay.
        source = [
            ".text", ".set noreorder", "probe:",
            "lbu\t$2,index", "sh\t$2,index", "j\t$31", "nop",
            f".extern\tindex, {size}",
        ]
        return MaspsxProcessor(source, sdata_limit=limit).process_lines()

    def test_external_gp_load_store_has_delay_nop(self):
        result = self.process()
        load = result.index("lbu\t$2,%gp_rel(index)($gp)")
        self.assertTrue(result[load + 1].startswith("nop"))
        self.assertEqual(result[load + 2], "sh\t$2,%gp_rel(index)($gp)")

    def test_g0_preserves_absolute_references(self):
        result = self.process(limit=0)
        self.assertIn("lbu\t$2,index", result)
        self.assertFalse(any("%gp_rel" in line for line in result))

    def test_external_object_larger_than_limit_stays_absolute(self):
        result = self.process(size=16)
        self.assertIn("lbu\t$2,index", result)
        self.assertFalse(any("%gp_rel" in line for line in result))

    def test_external_object_is_never_allocated(self):
        result = self.process()
        self.assertFalse(any(".space" in line or ".section .sbss" in line
                             for line in result))
        self.assertNotIn("index:", result)


class SdkReorderTests(unittest.TestCase):
    @unittest.skipUnless(shutil.which("mips-linux-gnu-as"), "MIPS assembler required")
    def test_macro_store_fills_return_slot(self):
        from decomp import Elf32
        source = [".text", ".globl probe", ".ent\tprobe", "probe:",
                  "li\t$2,1", "sh\t$2,0x80123456", "j\t$31", ".end\tprobe"]
        assembly = MaspsxProcessor(source, gas_reorder=True).process_lines()
        with tempfile.TemporaryDirectory() as directory:
            obj = Path(directory) / "probe.o"
            subprocess.run(["mips-linux-gnu-as", "-EL", "-march=r3000", "-O2",
                            "-no-pad-sections", "-o", str(obj), "-"],
                           input="\n".join(assembly) + "\n", text=True,
                           check=True, capture_output=True)
            self.assertEqual(Elf32(obj).read_symbol("probe"),
                             bytes.fromhex("010002241280013c0800e003563422a4"))

    def test_explicit_compiler_delay_slot_is_preserved(self):
        source = [".text", ".ent\tprobe", "probe:", ".set\tnoreorder",
                  "j\t$31", "addu\t$2,$4,$0", ".set\treorder", ".end\tprobe"]
        assembly = MaspsxProcessor(source, gas_reorder=True).process_lines()
        jump = assembly.index("j\t$31")
        self.assertEqual(assembly[jump + 1], "addu\t$2,$4,$0")
        self.assertIn(".set\tnoreorder", assembly)


if __name__ == "__main__":
    unittest.main()
