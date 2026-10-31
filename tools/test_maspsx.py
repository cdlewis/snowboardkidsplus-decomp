"""Regression checks for external small-data load hazards (no SDK needed)."""

import sys
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


if __name__ == "__main__":
    unittest.main()
