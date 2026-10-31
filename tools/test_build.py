"""Checks for selecting function-scoped compiler flags without changing peers."""

import unittest

from build import replace_compiler_function


class CompilerFunctionTests(unittest.TestCase):
    def test_only_requested_function_changes(self):
        normal = (".data\n.word 123\n.globl peer\n.ent peer\npeer:\n"
                  "normal_peer\n.end peer\n.globl reset\n.ent reset\n"
                  "reset:\nnormal_reset\n.end reset\n.extern index,2\n")
        alternate = normal.replace("normal_peer", "alternate_peer").replace(
            "normal_reset", "alternate_reset").replace(".word 123", ".word 456")
        self.assertEqual(replace_compiler_function(normal, alternate, "reset"),
                         normal.replace("normal_reset", "alternate_reset"))

    def test_missing_or_duplicate_function_is_rejected(self):
        region = ".ent reset\nreset:\nj $31\n.end reset\n"
        for normal, alternate in [(region, ""), ("", region),
                                  (region * 2, region), (region, region * 2)]:
            with self.subTest(normal=normal, alternate=alternate):
                with self.assertRaises(ValueError):
                    replace_compiler_function(normal, alternate, "reset")


if __name__ == "__main__":
    unittest.main()
