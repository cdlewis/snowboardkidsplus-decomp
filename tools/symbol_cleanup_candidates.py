#!/usr/bin/env python3
"""Find related symbol names and generic declarations worth reviewing in C."""

import re
from collections import Counter, defaultdict

from decomp import ROOT


SYMBOL = re.compile(r"^([A-Za-z_]\w*)\s*=\s*(0x[0-9a-fA-F]+);")
PLACEHOLDER = re.compile(r"(?:D|func)_([0-9a-fA-F]{8})$")
VAGUE = re.compile(r"[Uu]nknown|(?:^|[a-z_])Unk(?=[A-Z0-9_]|$)")
TOKEN = re.compile(r"\b[A-Za-z_]\w*\b")
# Strip literals and comments, but retain line breaks for declaration scanning.
NON_CODE = re.compile(r'/\*.*?\*/|//[^\n]*|"(?:\\.|[^"\\])*"|\'(?:\\.|[^\'\\])*\'', re.S)
GENERIC = re.compile(
    r"^\s*(?:extern\s+|static\s+)?(?:const\s+)?void\s*\*\s*(\w+)\b",
    re.M,
)

# Named functions with generic pointer parameters are worth reviewing too.
# Restrict the match to declaration-shaped lines, not arbitrary calls.
GENERIC_PARAMETER = re.compile(
    r"^\s*(?:extern\s+|static\s+)?[\w* ]+?\b(\w+)\s*\("
    r"[^;{}()]*\bvoid\s*\*[^;{}()]*\)\s*(?:;|\{)", re.M,
)
POINTER_CAST = re.compile(
    r"\(\s*(?:const\s+)?[A-Za-z_]\w*\s*\*\s*\)\s*&?\s*(\w+)\b"
)


def collect_symbols():
    addresses = {}
    for line in (ROOT / "symbol_addrs.txt").read_text().splitlines():
        if match := SYMBOL.match(line):
            addresses[match[1]] = int(match[2], 16)

    references = defaultdict(Counter)
    reasons = defaultdict(set)
    cast_counts = Counter()
    paths = sorted((*ROOT.glob("src/**/*.c"), *ROOT.glob("src/**/*.h"),
                    *ROOT.glob("include/**/*.h")))
    for path in paths:
        relative = path.relative_to(ROOT).as_posix()
        source = NON_CODE.sub(lambda m: "\n" * m[0].count("\n") or " ",
                              path.read_text())
        # A fallback alone provides no C evidence. Its callers still count.
        source = re.sub(r"^\s*INCLUDE_ASM[^\n]*", "", source, flags=re.M)
        for name, count in Counter(TOKEN.findall(source)).items():
            if match := PLACEHOLDER.fullmatch(name):
                addresses.setdefault(name, int(match[1], 16))
                reasons[name].add("address-based name")
            elif name in addresses and VAGUE.search(name):
                reasons[name].add("vague name")
            if name in addresses:
                references[name][relative] += count
        for match in GENERIC.finditer(source):
            name = match[1]
            if name in addresses:
                reasons[name].add("generic pointer declaration")

        for match in GENERIC_PARAMETER.finditer(source):
            name = match[1]
            if name in addresses:
                reasons[name].add("generic pointer parameter")
        for match in POINTER_CAST.finditer(source):
            name = match[1]
            # Casts on known globals can reveal an overly generic declaration.
            # Require repeated casts before flagging an otherwise named symbol.
            if name in addresses:
                cast_counts[name] += 1

    for name, count in cast_counts.items():
        if count >= 2:
            reasons[name].add("repeated pointer casts")

    # Include only symbols with implementation context, not header-only or
    # assembly-only references. A generic pointer may be perfectly legitimate.
    return {
        name: {"address": addresses[name], "reasons": sorted(why),
               "references": dict(references[name])}
        for name, why in reasons.items()
        if any(path.endswith(".c") for path in references[name])
    }


def cleanup_groups(max_size=8):
    symbols = collect_symbols()
    remaining = set(symbols)
    by_address = defaultdict(set)
    for name, info in symbols.items():
        by_address[info["address"]].add(name)

    def rank(name):
        info = symbols[name]
        return (len(info["reasons"]), "address-based name" in info["reasons"],
                sum(info["references"].values()))

    def affinity(left, right):
        a, b = symbols[left]["references"], symbols[right]["references"]
        return sum((4 if path.endswith(".c") else 1) * min(a[path], b[path], 5)
                   for path in a.keys() & b.keys())

    groups = []
    for seed in sorted(symbols, key=lambda n: (rank(n), -symbols[n]["address"], n),
                       reverse=True):
        if seed not in remaining:
            continue
        # Require a shared implementation, rather than grouping unrelated
        # globals merely because they are adjacent in memory.
        seed_files = {p for p in symbols[seed]["references"] if p.endswith(".c")}
        # Choose one common implementation as the anchor for the entire
        # group, so broadly referenced symbols cannot bridge unrelated units.
        def anchor_rank(path):
            neighbors = [n for n in remaining - {seed}
                         if path in symbols[n]["references"]]
            return (sum(sorted((affinity(seed, n) for n in neighbors),
                               reverse=True)[:max_size - 1]), path)

        anchor = max(seed_files, key=anchor_rank)
        peers = [n for n in remaining - {seed}
                 if anchor in symbols[n]["references"]
                 and symbols[n]["address"] != symbols[seed]["address"]]
        peers.sort(key=lambda n: (affinity(seed, n), rank(n),
                                  -symbols[n]["address"], n), reverse=True)
        group = [seed]
        used = {symbols[seed]["address"]}
        for name in peers:
            if symbols[name]["address"] not in used:
                group.append(name)
                used.add(symbols[name]["address"])
            if len(group) == max_size:
                break
        if len(group) < 2:
            continue
        for address in used:
            remaining.difference_update(by_address[address])
        groups.append([
            {"address": f"0x{symbols[n]['address']:08X}", "name": n,
             "reasons": symbols[n]["reasons"],
             "shared_context": anchor,
             "context": sorted(symbols[n]["references"],
                               key=lambda p: (not p.endswith(".c"),
                                              -symbols[n]["references"][p], p))[:4]}
            for n in sorted(group, key=lambda n: (symbols[n]["address"], n))
        ])
    return groups
