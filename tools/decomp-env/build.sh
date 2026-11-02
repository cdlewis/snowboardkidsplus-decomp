#!/usr/bin/env bash
set -euo pipefail
workspace="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
repo_root="$(cd "$workspace/project" && pwd -P)"
exec "$repo_root/.venv/bin/python" "$repo_root/tools/decomp_env.py" --workspace "$workspace" --build "${1:-base.c}"
