import tempfile
import unittest
from pathlib import Path

from clean_split_asm import clear_generated_asm
from build import is_function_disassembly


class CleanSplitAsmTests(unittest.TestCase):
    def test_build_excludes_function_disassemblies(self):
        self.assertTrue(is_function_disassembly(Path("asm/nonmatchings/unit/f.s")))
        self.assertTrue(is_function_disassembly(Path("asm/matchings/unit/f.s")))
        self.assertFalse(is_function_disassembly(Path("asm/data/unit.s")))

    def test_only_function_disassemblies_are_cleared(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            for name in ("nonmatchings", "matchings", "data"):
                path = root / "asm" / name
                path.mkdir(parents=True)
                (path / "generated.s").write_text("assembly")
            clear_generated_asm(root)
            self.assertFalse((root / "asm/nonmatchings").exists())
            self.assertFalse((root / "asm/matchings").exists())
            self.assertTrue((root / "asm/data/generated.s").exists())

    def test_refuses_symlinks(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            (root / "asm").mkdir()
            (root / "asm/nonmatchings").symlink_to(root, target_is_directory=True)
            with self.assertRaisesRegex(ValueError, "symlink"):
                clear_generated_asm(root)


if __name__ == "__main__":
    unittest.main()
