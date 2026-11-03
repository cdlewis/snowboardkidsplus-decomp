#!/usr/bin/env python3
"""PreToolUse protections shared by Codex and Claude.

These are mistake guards for visible tool arguments, not a shell sandbox.
Arbitrary programs and dynamically constructed paths cannot be fully inspected.
"""

import ast
import json
import re
import shlex
import sys
from pathlib import Path, PurePosixPath

from agent_build_policy import commands

ROOT = Path(__file__).resolve().parents[2]
# Reuse the commit validator's checksum parser.
sys.path.insert(0, str(ROOT / "tools"))
from validate_commit import checksum

CHECKSUM_FILES = {"tools/build.py", "snowboardkidsplus.yaml", "decomp.yaml"}
CHECKSUM_LINE = re.compile(r"TARGET_SHA1|\bsha1\s*:|3f1a7abc267ea29bafed1653121613ea467c1ac8", re.I)
READ_COMMANDS = {
    "cat", "head", "tail", "less", "more", "rg", "grep", "egrep", "fgrep",
    "ls", "stat", "wc", "sha1sum", "sha256sum", "file", "diff", "cmp",
    "objdump", "mipsel-linux-gnu-objdump", "readelf", "nm", "test", "[",
}


def relative_path(path, cwd=None):
    candidate = Path(path)
    if not candidate.is_absolute():
        candidate = Path(cwd or ROOT) / candidate
    try:
        return candidate.resolve().relative_to(ROOT).as_posix()
    except ValueError:
        return candidate.as_posix()


def file_reason(path):
    parts = PurePosixPath(path).parts
    name = PurePosixPath(path).name
    # Only explicitly registered source hasm units are authored assembly.
    # Generated asm/ output and all other assembly paths stay protected.
    if path.startswith("src/") and name.endswith(".s"):
        layout = (ROOT / "snowboardkidsplus.yaml").read_text()
        unit = re.escape(path[len("src/"):-2])
        if (re.search(r"^\s*hasm_in_src_path:\s*True\s*$", layout, re.M)
                and re.search(r"^\s*-\s*\[0x[0-9a-fA-F]+,\s*hasm,\s*"
                              + unit + r"\s*\]", layout, re.M)):
            return None
    if "asm" in parts or name.endswith((".s", ".S", ".asm")):
        return "assembly is generated; edit source/configuration and regenerate with ./tools/build-and-verify.sh"
    if name == "ctx.c":
        return "ctx.c is generated; edit its source headers instead"
    if name.endswith(".sha1"):
        return "reference SHA1 checksum files must not be edited"
    return None


def edit_reason(path, payload, cwd=None):
    path = relative_path(path, cwd)
    reason = file_reason(path)
    if reason:
        return reason
    if path not in CHECKSUM_FILES:
        return None
    if "content" in payload:
        try:
            if checksum(path, payload["content"].encode()) == checksum(path, (ROOT / path).read_bytes()):
                return None
        except (ValueError, SyntaxError, TypeError):
            pass
        return "the executable's expected SHA1 must not change"
    # Edit and MultiEdit supply old/new strings; patches supply changed lines.
    text = "\n".join(str(payload.get(key, "")) for key in ("old_string", "new_string", "changes"))
    if CHECKSUM_LINE.search(text) or payload.get("delete"):
        return "the executable's expected SHA1 must not change"
    return None


def patch_reason(patch, cwd=None):
    path = None
    changed = []
    deletion = False
    for line in patch.splitlines() + ["*** End Patch"]:
        match = re.match(r"\*\*\* (Add|Update|Delete) File: (.+)", line)
        move = re.match(r"\*\*\* Move to: (.+)", line)
        if match or move or line == "*** End Patch":
            if path:
                reason = edit_reason(path, {"changes": "\n".join(changed), "delete": deletion or bool(move)}, cwd)
                if reason:
                    return reason
            if match or move:
                path = match[2] if match else move[1]
                deletion = bool(move or match[1] == "Delete")
                changed = []
                reason = file_reason(relative_path(path, cwd))
                if reason:
                    return reason
        elif line.startswith(("+", "-")):
            changed.append(line[1:])
    return None


def references_protected(text, cwd=None):
    # Also recognize paths embedded in interpreter snippets and redirections.
    paths = re.findall(r"[\w./$*?{}\[\]-]+", text)
    return any(file_reason(relative_path(path, cwd))
               or relative_path(path, cwd) in CHECKSUM_FILES for path in paths)


def shell_reason(command, cwd=None):
    if "*** Begin Patch" in command:
        reason = patch_reason(command[command.index("*** Begin Patch"):], cwd)
        if reason:
            return reason
        # A patch payload is not a shell program. Keep inspecting real shell
        # commands that merely contain a patch string or have trailing commands.
        if (command.lstrip().startswith("*** Begin Patch\n")
                and command.rstrip().endswith("*** End Patch")):
            return None
    lexer = shlex.shlex(command, posix=True, punctuation_chars=";&|()<>\n")
    lexer.whitespace_split = True
    lexer.whitespace = " \t\r"
    try:
        tokens = list(lexer)
    except ValueError:
        return "cannot inspect malformed shell syntax"
    segments = []
    segment = []
    for token in tokens + [";"]:
        if token and all(c in ";&|()\n" for c in token):
            if segment:
                segments.append(segment)
            segment = []
        else:
            segment.append(token)
    for args in segments:
        while args and ("=" in args[0] or args[0] in {"env", "command", "exec", "sudo", "then", "do", "!"}):
            args = args[1:]
        if not args:
            continue
        name = PurePosixPath(args[0]).name
        if name == "git":
            if any(arg == "--no-verify" or arg.startswith("--no-verify=") for arg in args):
                return "do not bypass Git hooks with --no-verify"
            if "commit" in args and any(re.fullmatch(r"-[a-zA-Z]*n[a-zA-Z]*", arg) for arg in args):
                return "git commit -n bypasses hooks; run the validation hook"
            if any("core.hookspath" in arg.lower() for arg in args):
                return "do not override core.hooksPath to bypass validation"
        if name in {"sh", "bash", "zsh"}:
            for index, arg in enumerate(args[1:-1], 1):
                if arg.startswith("-") and "c" in arg:
                    reason = shell_reason(args[index + 1], cwd)
                    if reason:
                        return reason
        text = " ".join(args[1:])
        if not references_protected(text, cwd):
            continue
        writes = any(">" in arg and set(arg) <= set("<>&") for arg in args)
        read_only = name in READ_COMMANDS
        if name == "sed":
            read_only = not any(arg.startswith("-i") or arg == "--in-place" or arg.startswith("--in-place=") for arg in args[1:])
            # sed's w/e commands can write files or execute commands.
            read_only = read_only and not re.search(r"[;\s][we]\s|^[we]\s|/[^/]*w(?:\s|$)", text)
        if name == "git":
            read_only = any(arg in {"diff", "show", "log", "status", "ls-files", "grep"} for arg in args[1:])
        if writes or not read_only:
            return "shell writes to assembly, ctx.c, or checksum sources are blocked; use source/configuration edits (preserving SHA1 fields)"
    return None


def source_reason(source, cwd=None):
    """Inspect literal shell/patch arguments in Codex orchestration calls."""
    literal = r'''("(?:\\.|[^"\\])*"|'(?:\\.|[^'\\])*'|`[^`]*`)'''
    pattern = r'''(?:["']?(?:cmd|command)["']?\s*:|(?:tools\.)?apply_patch\s*\()\s*'''
    for match in re.finditer(pattern + literal, source, re.S):
        raw = match[1]
        try:
            value = raw[1:-1] if raw.startswith("`") else ast.literal_eval(raw)
        except (SyntaxError, ValueError):
            continue
        reason = patch_reason(value, cwd) if "*** Begin Patch" in value else shell_reason(value, cwd)
        if reason:
            return reason
    return None


def event_reason(event, tool_name="", cwd=None):
    if isinstance(event, str):
        if tool_name in {"functions.exec", "exec"}:
            return source_reason(event, cwd)
        if "*** Begin Patch" in event:
            return patch_reason(event, cwd)
        return None
    if isinstance(event, list):
        for item in event:
            reason = event_reason(item, tool_name, cwd)
            if reason:
                return reason
    if not isinstance(event, dict):
        return None
    tool_name = event.get("tool_name", tool_name).lower()
    cwd = event.get("cwd", event.get("workdir", cwd))
    path = event.get("file_path", event.get("path"))
    editing = any(key in event for key in ("content", "old_string", "new_string", "edits")) or any(
        name in tool_name for name in ("write", "edit", "patch"))
    if path and editing:
        edits = event.get("edits", [event])
        for edit in edits:
            reason = edit_reason(path, edit, cwd)
            if reason:
                return reason
    for value in event.values():
        reason = event_reason(value, tool_name, cwd)
        if reason:
            return reason
    for command in commands(event):
        reason = shell_reason(command, cwd)
        if reason:
            return reason
    return None


def main():
    try:
        reason = event_reason(json.load(sys.stdin))
    except (ValueError, TypeError, OSError) as error:
        reason = f"could not inspect hook input: {error}"
    if reason:
        print(f"Blocked: {reason}.", file=sys.stderr)
        return 2
    return 0


if __name__ == "__main__":
    sys.exit(main())
