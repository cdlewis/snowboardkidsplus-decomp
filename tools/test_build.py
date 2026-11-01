"""Checks for selecting function-scoped compiler flags without changing peers."""

import unittest
from pathlib import Path

from build import replace_compiler_function, select_function_compiler


class CompilerFunctionTests(unittest.TestCase):
    def test_comparison_preserves_normal_function_compiler(self):
        unit = Path("src/syslib/libcd/sys.c")
        base, sdk = Path("base-cc1"), Path("sdk-cc1")
        overrides = {"function_compilers": {"CdComstr": sdk}}
        self.assertEqual(select_function_compiler(unit, "CdComstr", base, overrides, False), sdk)
        self.assertEqual(select_function_compiler(unit, "CdComstr", base, overrides, True), sdk)
        self.assertEqual(select_function_compiler(unit, "other", base, overrides, True), base)

    def test_game_function_cannot_select_another_compiler(self):
        with self.assertRaises(ValueError):
            select_function_compiler(Path("src/main.c"), "func", Path("base-cc1"),
                                     {"function_compilers": {"func": Path("other-cc1")}}, False)

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

    def test_colliding_compiler_labels_are_scoped_to_replacement(self):
        normal = ".ent peer\npeer:\n$L91:\n.end peer\n.ent reset\nreset:\n.end reset\n"
        alternate = ".ent reset\nreset:\nb $L91\n$L91:\n.end reset\n"
        result = replace_compiler_function(normal, alternate, "reset")
        self.assertIn("$L91:\n.end peer", result)
        self.assertIn("b $L91_reset\n$L91_reset:", result)


if __name__ == "__main__":
    unittest.main()
