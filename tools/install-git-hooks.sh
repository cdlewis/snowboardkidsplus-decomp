#!/bin/sh
set -eu

repo_root=$(git rev-parse --show-toplevel)
cd "$repo_root"
current_path=$(git config --get core.hooksPath || true)
if [ -n "$current_path" ] && [ "$current_path" != tools/git-hooks ]; then
    echo "Existing core.hooksPath is '$current_path'; merge the project pre-commit hook there instead of replacing it." >&2
    exit 1
fi
if [ ! -x tools/git-hooks/pre-commit ]; then
    echo "tools/git-hooks/pre-commit is missing or not executable." >&2
    exit 1
fi
git config --local core.hooksPath tools/git-hooks
echo "Installed repository hooks from tools/git-hooks."
