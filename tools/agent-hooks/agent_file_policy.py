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


def python_reason(source, cwd=None):
    """Check visible file mutations, without treating read paths as destinations."""
    try:
        tree = ast.parse(source)
    except SyntaxError:
        return "cannot inspect malformed Python syntax"
    bindings = {}
    path_objects = set()

    def is_path_object(node):
        if isinstance(node, ast.Name):
            return node.id in path_objects
        if isinstance(node, ast.Call):
            name = node.func.id if isinstance(node.func, ast.Name) else getattr(node.func, "attr", "")
            return name in {"Path", "PurePath"}
        return False

    def path(node):
        if isinstance(node, ast.Constant) and isinstance(node.value, str):
            return node.value
        if isinstance(node, ast.Name):
            return bindings.get(node.id)
        if isinstance(node, ast.Call):
            name = node.func.id if isinstance(node.func, ast.Name) else getattr(node.func, "attr", "")
            if name in {"Path", "PurePath", "open"} and node.args:
                return path(node.args[0])
        return None

    for node in ast.walk(tree):
        if isinstance(node, ast.Assign):
            for target in node.targets:
                if isinstance(target, ast.Name):
                    bindings[target.id] = path(node.value)
                    if is_path_object(node.value):
                        path_objects.add(target.id)
    for node in ast.walk(tree):
        if not isinstance(node, ast.Call):
            continue
        name = node.func.id if isinstance(node.func, ast.Name) else getattr(node.func, "attr", "")
        targets = []
        if name == "open":
            mode_index = 0 if isinstance(node.func, ast.Attribute) else 1
            mode = node.args[mode_index] if len(node.args) > mode_index else next(
                (kw.value for kw in node.keywords if kw.arg == "mode"), ast.Constant("r"))
            mode = path(mode)
            if mode is None or any(flag in mode for flag in "wax+"):
                targets.append(path(node.func.value) if isinstance(node.func, ast.Attribute)
                               else path(node.args[0]) if node.args else None)
        elif isinstance(node.func, ast.Attribute) and (
                name in {"write_text", "write_bytes", "touch"}
                or name in {"unlink", "rename", "replace"} and is_path_object(node.func.value)):
            targets.append(path(node.func.value))
            if name in {"rename", "replace"} and node.args:
                targets.append(path(node.args[0]))
        elif name in {"remove", "unlink", "rename", "replace"} and (
                isinstance(node.func, ast.Name) or isinstance(node.func.value, ast.Name)
                and node.func.value.id in {"os", "posix"}):
            targets.extend(path(arg) for arg in node.args)
        elif name in {"system", "run", "call", "check_call", "check_output", "Popen"} and node.args:
            try:
                command = ast.literal_eval(node.args[0])
            except (ValueError, TypeError):
                command = None
            if isinstance(command, (list, tuple)) and all(isinstance(arg, str) for arg in command):
                command = shlex.join(command)
            if isinstance(command, str):
                reason = shell_reason(command, cwd)
                if reason:
                    return reason
            elif references_protected(source, cwd):
                return "cannot resolve Python subprocess command referencing protected files"
        for target in targets:
            if target is not None and references_protected(target, cwd):
                return "Python writes to protected files require source/configuration patches preserving SHA1 fields"
            if target is None and references_protected(source, cwd):
                return "cannot resolve Python write destination; use an explicit path or a structured patch"
    return None


def shell_tokens(command):
    lexer = shlex.shlex(command, posix=True, punctuation_chars=";&|()<>\n")
    lexer.whitespace_split = True
    lexer.whitespace = " \t\r"
    return list(lexer)


def heredoc_source(command, cwd=None):
    """Separate shell syntax from literal stdin, including multiple heredocs."""
    lines = command.splitlines(keepends=True)
    clean = []
    index = 0
    while index < len(lines):
        header = lines[index]
        clean.append(header)
        index += 1
        try:
            tokens = shell_tokens(header)
        except ValueError:
            continue  # The complete shell lexer handles multiline quotes.
        for pos, token in enumerate(tokens[:-1]):
            if token != "<<":
                continue
            delimiter = tokens[pos + 1]
            strip_tabs = delimiter.startswith("-")
            if strip_tabs:
                delimiter = delimiter[1:]
            body = []
            while index < len(lines):
                line = lines[index]
                index += 1
                check = line.lstrip("\t") if strip_tabs else line
                if check.rstrip("\r\n") == delimiter:
                    break
                body.append(check)
            else:
                return command, "unterminated shell heredoc"
            source = "".join(body)
            # Expanded heredocs can execute shell substitutions before stdin is
            # passed to the command. Require quoting for those payloads.
            quoted = re.search(r"<<-?\s*(['\"])" + re.escape(delimiter), header)
            if not quoted and ("$(" in source or "`" in source):
                return command, "quote the heredoc delimiter to prevent shell command substitution"
            start = pos - 1
            while start >= 0 and tokens[start] not in {";", "&&", "||", "|"}:
                start -= 1
            args = tokens[start + 1:pos]
            while args and ("=" in args[0] or args[0] in {"env", "command", "exec"}):
                args = args[1:]
            name = PurePosixPath(args[0]).name if args else ""
            reason = None
            if name.startswith("python") and "-" in args:
                reason = python_reason(source, cwd)
            elif name in {"sh", "bash", "zsh"}:
                reason = shell_reason(source, cwd)
            if reason:
                return command, reason
    return "".join(clean), None


def git_hooks_override(args):
    for index, arg in enumerate(args):
        value = args[index + 1] if arg in {"-c", "--config-env"} and index + 1 < len(args) else ""
        if arg.startswith(("--config-env=", "-c")) and arg not in {"-c", "--config-env"}:
            value = arg.split("=", 1)[1] if arg.startswith("--config-env=") else arg[2:]
        if value.lower().startswith("core.hookspath="):
            return True
    if "config" not in args:
        return False
    options = args[args.index("config") + 1:]
    if not any(arg.lower() == "core.hookspath" for arg in options):
        return False
    if any(arg in {"--get", "--get-all", "--get-regexp", "--get-urlmatch", "--list", "get", "list"} for arg in options):
        return False
    # A single key is Git's legacy read syntax. Other forms mutate configuration.
    return len(options) != 1 or options[0].lower() != "core.hookspath"


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
    command, reason = heredoc_source(command, cwd)
    if reason:
        return reason
    try:
        tokens = shell_tokens(command)
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
            if git_hooks_override(args):
                return "do not override core.hooksPath to bypass validation"
        if name in {"sh", "bash", "zsh"}:
            for index, arg in enumerate(args[1:-1], 1):
                if arg.startswith("-") and "c" in arg:
                    reason = shell_reason(args[index + 1], cwd)
                    if reason:
                        return reason
        if name.startswith("python") and "-c" in args:
            index = args.index("-c")
            if index + 1 < len(args):
                reason = python_reason(args[index + 1], cwd)
                if reason:
                    return reason
                continue
        # Redirection writes affect the destination, not every input path.
        redirected = set()
        for index, arg in enumerate(args[:-1]):
            if ">" in arg and set(arg) <= set("<>&|"):
                if references_protected(args[index + 1], cwd):
                    return "shell redirection writes to a protected file"
                redirected.update({index, index + 1})
        args = [arg for index, arg in enumerate(args) if index not in redirected]
        text = " ".join(args[1:])
        if not references_protected(text, cwd):
            continue
        read_only = name in READ_COMMANDS
        if name == "cp" and args:
            destination = args[-1]
            for index, arg in enumerate(args[1:], 1):
                if arg in {"-t", "--target-directory"} and index + 1 < len(args):
                    destination = args[index + 1]
                elif arg.startswith("--target-directory="):
                    destination = arg.split("=", 1)[1]
                elif arg.startswith("-t") and len(arg) > 2:
                    destination = arg[2:]
            if not references_protected(destination, cwd):
                continue
        if name == "sed":
            read_only = not any(arg.startswith("-i") or arg == "--in-place" or arg.startswith("--in-place=") for arg in args[1:])
            # sed's w/e commands can write files or execute commands.
            read_only = read_only and not re.search(r"[;\s][we]\s|^[we]\s|/[^/]*w(?:\s|$)", text)
        if name == "git":
            read_only = any(arg in {"diff", "show", "log", "status", "ls-files", "grep", "config"} for arg in args[1:])
            # Staging updates the index, without writing protected worktree files.
            read_only = read_only or (len(args) > 1 and args[1] == "add")
        if not read_only:
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
