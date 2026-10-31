"""Exercise the commit gate in disposable Git repos without any game data."""

import hashlib
import os
import shutil
import subprocess
import sys
import tempfile
import unittest
from unittest.mock import patch
from pathlib import Path

from validate_commit import check_inputs, validate

TOOLS = Path(__file__).resolve().parent
PAYLOAD = b"synthetic executable fixture\n"
SHA1 = hashlib.sha1(PAYLOAD).hexdigest()


class CommitValidationTests(unittest.TestCase):
    def setUp(self):
        # Hooks export repository/index settings. A fixture must never inherit
        # an active commit's index (especially an absolute temporary index).
        local_names = subprocess.check_output(
            ["git", "rev-parse", "--local-env-vars"], text=True).splitlines()
        environment = patch.dict(os.environ, {
            key: value for key, value in os.environ.items() if key not in local_names
        }, clear=True)
        environment.start()
        self.addCleanup(environment.stop)
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        self.git("init", "-q")
        self.git("config", "user.name", "Validation test")
        self.git("config", "user.email", "validation@example.invalid")
        # Isolate fixtures from any global hooks, then let installation be tested.
        self.git("config", "core.hooksPath", "/dev/null")
        self.write("tools/build.py", f'TARGET_SHA1 = "{SHA1}"\n')
        self.write("snowboardkidsplus.yaml", f"sha1: {SHA1}\n")
        self.write("decomp.yaml", f"versions:\n  - name: jp\n    sha1: {SHA1}\n")
        self.write("src/probe.c", "int probe(void) { return 1; }\n")
        self.write("disc/SLPS_018.23", PAYLOAD)
        self.write(".gitignore", "disc/\nbuild/\n.venv/\nexpected/\n")
        self.write("Makefile", """.PHONY: split check all
split:
	@echo split >> validation.log
check:
	@echo check >> validation.log
all:
	@echo all >> validation.log
	@mkdir -p build
	@cp disc/SLPS_018.23 build/slps_018.23.exe
""")
        for name in ("validate_commit.py", "git-hooks/pre-commit", "install-git-hooks.sh"):
            destination = self.root / "tools" / name
            destination.parent.mkdir(parents=True, exist_ok=True)
            shutil.copy2(TOOLS / name, destination)
        self.git("add", ".")
        self.git("commit", "-qm", "Fixture baseline")

    def git(self, *args):
        return subprocess.check_output(["git", *args], cwd=self.root, stderr=subprocess.PIPE)

    def write(self, name, contents):
        path = self.root / name
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_bytes(contents if isinstance(contents, bytes) else contents.encode())

    def test_clean_inputs_allow_unrelated_local_files(self):
        self.write(".envrc", "local configuration is not a build input\n")
        self.write("notes.md", "unfinished notes\n")
        validate(self.root)
        self.assertEqual((self.root / "validation.log").read_text().splitlines(),
                         ["split", "check", "all"])

    def test_partial_staging_fails_before_build(self):
        self.write("src/probe.c", "int probe(void) { return 2; }\n")
        self.git("add", "src/probe.c")
        self.write("src/probe.c", "int probe(void) { return 1; }\n")
        with self.assertRaisesRegex(ValueError, "differ from the index"):
            validate(self.root)
        self.assertFalse((self.root / "validation.log").exists())

    def test_untracked_header_blocks_validation(self):
        self.write("include/new.h", "#define CHANGED 1\n")
        with self.assertRaisesRegex(ValueError, "include/new.h"):
            check_inputs(self.root)

    def test_agent_checksum_edits_are_rejected_in_all_locations(self):
        for path in ("tools/build.py", "snowboardkidsplus.yaml", "decomp.yaml"):
            with self.subTest(path=path):
                original = (self.root / path).read_text()
                self.write(path, original.replace(SHA1, "0" * 40))
                self.git("add", path)
                with self.assertRaisesRegex(ValueError, "expected checksum changed"):
                    check_inputs(self.root)
                self.write(path, original)
                self.git("add", path)

    def test_generated_outputs_cannot_be_staged(self):
        self.write("expected/probe.o", b"generated fixture")
        self.git("add", "-f", "expected/probe.o")
        with self.assertRaisesRegex(ValueError, "generated outputs"):
            check_inputs(self.root)

    def test_regression_failure_stops_before_build(self):
        makefile = (self.root / "Makefile").read_text().replace(
            "@echo check >> validation.log", "@echo check >> validation.log\n\t@false")
        self.write("Makefile", makefile)
        self.git("add", "Makefile")
        with self.assertRaises(subprocess.CalledProcessError):
            validate(self.root)
        self.assertEqual((self.root / "validation.log").read_text().splitlines(), ["split", "check"])

    def test_successful_build_with_wrong_image_still_fails(self):
        makefile = (self.root / "Makefile").read_text().replace(
            "@cp disc/SLPS_018.23 build/slps_018.23.exe",
            "@echo wrong > build/slps_018.23.exe")
        self.write("Makefile", makefile)
        self.git("add", "Makefile")
        with self.assertRaisesRegex(ValueError, "differs from protected checksum"):
            validate(self.root)

    def test_altering_disc_and_build_together_still_fails(self):
        self.write("disc/SLPS_018.23", b"changed original")
        with self.assertRaisesRegex(ValueError, "disc/SLPS_018.23: SHA1"):
            validate(self.root)

    def test_installed_hook_blocks_real_git_commit(self):
        self.git("config", "--unset", "core.hooksPath")
        self.write(".venv/placeholder", "")
        python = self.root / ".venv/bin/python"
        python.parent.mkdir(parents=True, exist_ok=True)
        python.symlink_to(sys.executable)
        subprocess.run([str(self.root / "tools/install-git-hooks.sh")], cwd=self.root,
                       check=True, capture_output=True)
        self.write("src/probe.c", "int probe(void) { return 3; }\n")
        self.git("add", "src/probe.c")
        self.git("commit", "-qm", "Validated fixture")
        before = self.git("rev-parse", "HEAD")
        self.write("tools/build.py", f'TARGET_SHA1 = "{"0" * 40}"\n')
        self.git("add", "tools/build.py")
        result = subprocess.run(["git", "commit", "-qm", "Rejected fixture"],
                                cwd=self.root, capture_output=True, text=True)
        self.assertNotEqual(result.returncode, 0)
        self.assertIn("expected checksum changed", result.stderr)
        self.assertEqual(self.git("rev-parse", "HEAD"), before)


if __name__ == "__main__":
    unittest.main()
