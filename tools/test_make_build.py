"""Exercise the Make dependency graph with real tools in an isolated directory."""

import shutil
import subprocess
import tempfile
import time
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
CC1 = ROOT / "tools/gcc/gcc-2.8.1-psx/cc1"


@unittest.skipUnless(CC1.exists() and shutil.which("mips-linux-gnu-as"),
                     "install the PSX toolchain before running Make integration tests")
class MakeBuildTests(unittest.TestCase):
    def setUp(self):
        self.workspace = tempfile.TemporaryDirectory(prefix="skp-make-test-")
        self.addCleanup(self.workspace.cleanup)
        self.root = Path(self.workspace.name)
        for directory in ("src", "include", "config", "tools"):
            (self.root / directory).mkdir()
        for filename in ("Makefile", "config/compiler.mk", "tools/merge_functions.py"):
            shutil.copy2(ROOT / filename, self.root / filename)
        (self.root / "src/value.h").write_text("#define VALUE 1\n")
        (self.root / "src/alpha.c").write_text(
            '#include "value.h"\nint alpha(void) { return VALUE; }\n')
        (self.root / "src/beta.c").write_text("int beta(void) { return 7; }\n")
        (self.root / "src/packet.c").write_text(
            '__asm__(".include \\"packet.inc\\" # maspsx-keep");\n')
        (self.root / "packet.inc").write_text(".section .rodata\n.word 1\n.text\n")
        self.command = [
            "make", "--no-print-directory", "-s", "-j4", "scratch",
            "COMPARISON=1", "BUILD_DIR=objects",
            f"SKP_CC1={CC1}",
            f"SKP_MASPSX={ROOT / 'tools/maspsx/maspsx.py'}",
            f"PY={ROOT / '.venv/bin/python'}",
        ]

    def run_make(self, *arguments, success=True):
        result = subprocess.run(self.command + list(arguments), cwd=self.root,
                                text=True, capture_output=True)
        if success:
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        else:
            self.assertNotEqual(result.returncode, 0)
        return result

    def timestamps(self):
        return {p.relative_to(self.root).as_posix(): p.stat().st_mtime_ns
                for p in (self.root / "objects").rglob("*") if p.is_file()}

    def test_noop_and_header_dependency(self):
        self.run_make()
        before = self.timestamps()
        self.run_make()
        self.assertEqual(before, self.timestamps())
        time.sleep(0.02)
        (self.root / "src/value.h").write_text("#define VALUE 2\n")
        self.run_make()
        after = self.timestamps()
        self.assertGreater(after["objects/src/alpha.o"], before["objects/src/alpha.o"])
        self.assertEqual(after["objects/src/beta.o"], before["objects/src/beta.o"])
        self.assertEqual(after["objects/src/packet.o"], before["objects/src/packet.o"])

    def test_assembler_include_rebuilds_only_assembly(self):
        self.run_make()
        before = self.timestamps()
        time.sleep(0.02)
        (self.root / "packet.inc").write_text(".section .rodata\n.word 2\n.text\n")
        self.run_make()
        after = self.timestamps()
        self.assertGreater(after["objects/src/packet.o"], before["objects/src/packet.o"])
        for suffix in (".i", ".base.s", ".s", ".masp.s"):
            path = "objects/src/packet" + suffix
            self.assertEqual(after[path], before[path], path)
        self.assertEqual(after["objects/src/alpha.o"], before["objects/src/alpha.o"])

    def test_changed_flags_rebuild_and_compiler_failure_propagates(self):
        self.run_make()
        before = self.timestamps()
        self.run_make("SKP_G_VALUE=8")
        after = self.timestamps()
        for unit in ("alpha", "beta", "packet"):
            key = f"objects/src/{unit}.o"
            self.assertGreater(after[key], before[key])
        self.run_make("EXTRA_CC_FLAGS=-this-option-does-not-exist", success=False)
        self.run_make()
        before = self.timestamps()
        self.run_make()
        self.assertEqual(before, self.timestamps())

    def test_comparison_cannot_use_image_directory_or_verify(self):
        result = self.run_make("BUILD_DIR=build", success=False)
        self.assertIn("separate BUILD_DIR", result.stderr)
        self.run_make("BUILD_DIR=./build", success=False)
        self.assertFalse((self.root / "build").exists())
        self.run_make("verify", success=False)
        self.assertFalse((self.root / "build/slps_018.23.exe").exists())


if __name__ == "__main__":
    unittest.main()
