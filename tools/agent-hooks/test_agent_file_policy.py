#!/usr/bin/env python3
"""Exercise hook payloads without changing protected files or running Git writes."""

import json
import subprocess
import sys
import unittest

from agent_file_policy import ROOT, event_reason


class FilePolicyTests(unittest.TestCase):
    def check_cases(self, blocked, events):
        for event in events:
            with self.subTest(event=event):
                self.assertEqual(bool(event_reason(event)), blocked)

    def test_protected_edit_payloads(self):
        self.check_cases(True, [
            {"tool_name": "Edit", "tool_input": {"file_path": path,
             "old_string": "old", "new_string": "new"}}
            for path in ("asm/nonmatchings/test.s", str(ROOT / "asm/test.s"),
                         "src/../asm/test.s", "ctx.c", "nonmatchings/test/ctx.c",
                         "target.sha1", "scratch/test.S", "scratch/test.asm")
        ])
        self.check_cases(True, [
            {"tool_name": "Write", "tool_input": {"file_path": "ctx.c", "content": ""}},
            {"arguments": {"path": "asm/test.s", "content": "text"}},
            {"tool_name": "MultiEdit", "tool_input": {"file_path": "tools/build.py", "edits": [
                {"old_string": "TARGET_SHA1 = 'old'", "new_string": "TARGET_SHA1 = 'new'"}]}},
        ])

    def test_checksum_fields(self):
        for path in ("tools/build.py", "snowboardkidsplus.yaml", "decomp.yaml"):
            current = (ROOT / path).read_text()
            self.check_cases(False, [{"tool_name": "Write", "tool_input": {
                "file_path": path, "content": current + "\n# harmless comment\n"}}])
            self.check_cases(True, [{"tool_name": "Write", "tool_input": {
                "file_path": path, "content": current.replace(
                    "3f1a7abc267ea29bafed1653121613ea467c1ac8", "0" * 40)}}])
        self.check_cases(True, [
            {"tool_name": "Edit", "tool_input": {"file_path": "decomp.yaml",
             "old_string": "sha1: old", "new_string": "sha1: new"}},
        ])

    def test_patch_formats(self):
        patches = [
            "*** Update File: asm/test.s\n@@\n-old\n+new",
            "*** Add File: ctx.c\n+text",
            "*** Delete File: tools/build.py",
            "*** Update File: tools/build.py\n*** Move to: tools/other.py",
            "*** Update File: src/test.c\n*** Move to: asm/test.s\n@@\n-old\n+new",
            "*** Update File: snowboardkidsplus.yaml\n@@\n-sha1: old\n+sha1: new",
        ]
        for body in patches:
            patch = "*** Begin Patch\n" + body + "\n*** End Patch"
            self.check_cases(True, [
                {"tool_name": "apply_patch", "tool_input": patch},
                {"tool_name": "apply_patch", "arguments": {"input": patch}},
                {"tool_name": "functions.exec", "tool_input":
                    "await tools.apply_patch(" + json.dumps(patch) + ");"},
            ])

    def test_shell_writes_and_hook_bypasses(self):
        commands = [
            'git commit --no-verify -m test', 'git -C . commit -m test --no-verify',
            'git commit -n', 'git commit -an', 'git push --no-verify',
            'env X=1 /usr/bin/git commit --no-verify',
            "bash -lc 'git commit --no-verify'", 'true && git commit --no-verify',
            'git -c core.hooksPath=/dev/null commit -m test',
            'sed -i s/old/new/ asm/test.s', 'echo text >ctx.c',
            'cat src/test.c > asm/test.s', 'tee asm/test.s',
            'rm ctx.c', 'mv src/test.c asm/test.s', 'cp src/test.c target.sha1',
            'python3 -c "open(\'ctx.c\', \'w\').write(\'text\')"',
            'sed -i s/old/new/ tools/build.py',
            f'rm {ROOT}/tools/build.py',
            'sed "w asm/test.s" src/test.c',
        ]
        for command in commands:
            self.check_cases(True, [
                {"tool_name": "Bash", "tool_input": {"command": command}},
                {"params": {"cmd": command}},
                {"tool_name": "functions.exec", "tool_input":
                    "await tools.exec_command({cmd: " + json.dumps(command) + "});"},
            ])

    def test_allowed_workflows(self):
        self.check_cases(False, [
            {"tool_name": "Read", "tool_input": {"file_path": "asm/test.s"}},
            {"tool_name": "Read", "tool_input": {"file_path": "ctx.c"}},
            {"tool_name": "Edit", "tool_input": {"file_path": "src/test.c",
             "old_string": "old", "new_string": "new"}},
            {"tool_name": "apply_patch", "tool_input": "*** Begin Patch\n*** Update File: decomp.yaml\n@@\n-# old\n+# new\n*** End Patch"},
        ])
        self.check_cases(False, [{"tool_input": {"cmd": command}} for command in (
            './tools/build-and-verify.sh --expected --objects',
            'python3 tools/match_function.py example nonmatchings/example/attempt.c',
            'cat asm/nonmatchings/test.s', 'rg name asm/',
            "sed -n '1,100p' asm/test.s", 'git diff -- asm/test.s',
            'git show HEAD:tools/build.py', 'sha1sum disc/SLPS_018.23',
            'git commit -m test', 'git status --short',
            'echo "git commit --no-verify"',
        )])

    def test_process_exit_codes_and_configuration(self):
        for event, code in (({"tool_input": {"cmd": "git commit --no-verify"}}, 2),
                            ({"tool_input": {"cmd": "git status"}}, 0)):
            result = subprocess.run([sys.executable, str(ROOT / 'tools/agent-hooks/agent_file_policy.py')],
                                    input=json.dumps(event), text=True, capture_output=True)
            self.assertEqual(result.returncode, code, result.stderr)
            self.assertEqual('Blocked:' in result.stderr, code == 2)
        for filename in ('.codex/hooks.json', '.claude/settings.json'):
            config = json.loads((ROOT / filename).read_text())
            self.assertTrue(any('agent_file_policy.py' in hook['command']
                                for entry in config['hooks']['PreToolUse']
                                for hook in entry['hooks']))


if __name__ == '__main__':
    unittest.main()
