#!/usr/bin/env python3
"""Find source-backed library leads by bytes, structure, and names independently.

Examples:
  python3 tools/find_library_matches.py --include-game --unattempted
  python3 tools/find_library_matches.py --reference ../game/asm/matchings ../game/src
  python3 tools/find_library_matches.py --source ../sdk/src --json > /tmp/leads.json

Sibling psy-q-decomp, psyz and sotn-decomp sources are discovered automatically.
Additional --reference pairs use the same unit mapping as rank_library_functions.
Source-only hits are leads, never byte-match claims. No reference is compiled and
no project names, sources, generated files, or match status are changed.
"""

import argparse
from collections import Counter
from difflib import SequenceMatcher
import json
from pathlib import Path
import re
import struct
import subprocess

from decomp import ROOT, PsxImage, active_functions, require_fresh, uncomment
from rank_library_functions import instruction_tokens, load_references, source_definition


def canonical_name(name):
    # Preserve semantic suffixes and digits; only tolerate leading underscores/case.
    return name.lstrip('_').casefold()


def binary_tokens(function):
    """Mask address operands, not constants, registers, or local branch distances.

    This is an instruction heuristic, NOT an object relocation-table mask. A
    masked equality is deliberately labelled address-masked, never linked-exact.
    """
    count = len(instruction_tokens(function))
    words = struct.unpack('<' + 'I' * count, function.data[:count * 4])
    result = []
    for word, instruction in zip(words, function.instructions):
        if word >> 26 in (2, 3):  # J / JAL absolute target
            word &= 0xFC000000
        elif re.search(r'%(?:hi|lo|gp_rel)\(', instruction):
            word &= 0xFFFF0000
        result.append(word)
    return tuple(result)


def ratio(left, right):
    return SequenceMatcher(None, left, right, autojunk=False).ratio()


def provenance(path):
    """Record the checkout revision without contacting or modifying its remote."""
    def git(*args):
        run = subprocess.run(['git', '-C', str(path.parent), *args],
                             capture_output=True, text=True)
        return run.stdout.strip() if run.returncode == 0 else None
    revision = git('rev-parse', 'HEAD')
    remote = git('config', '--get', 'remote.origin.url')
    top = git('rev-parse', '--show-toplevel')
    url = None
    if remote and revision and top:
        remote = re.sub(r'^git@github.com:', 'https://github.com/', remote)
        if remote.startswith('https://github.com/'):
            url = f'{remote.removesuffix(".git")}/blob/{revision}/{path.resolve().relative_to(Path(top).resolve())}'
    return {'revision': revision, 'url': url}


def source_index(roots):
    result = {}
    seen = set()
    for root in roots:
        if not root.is_dir():
            raise ValueError(f'source directory does not exist: {root}')
        for path in sorted(root.rglob('*')):
            if path.suffix.lower() not in ('.c', '.s') or path.resolve() in seen:
                continue
            seen.add(path.resolve())
            text = path.read_text(errors='replace')
            if path.suffix.lower() == '.c':
                names = set(re.findall(r'\b([A-Za-z_]\w*)\s*\([^;{}]*\)\s*\{', uncomment(text)))
                definitions = [(name, source_definition(text, name)) for name in names]
                kind = 'C'
            else:
                # Assembly source is a pointer for provenance review, not hasm eligibility.
                definitions = [(m[1], text.count('\n', 0, m.start()) + 1)
                               for m in re.finditer(r'^\s*(?:glabel\s+|\.globl\s+|\.global\s+)(\w+)', text, re.M)]
                kind = 'assembly (provenance required)'
            for name, line in definitions:
                if line:
                    result.setdefault(canonical_name(name), []).append(
                        {'name': name, 'source': str(path.resolve()), 'line': line, 'kind': kind})
    return result


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument('--reference', nargs=2, action='append', type=Path, default=[], metavar=('MATCHING_ASM', 'SOURCE'))
    parser.add_argument('--source', action='append', type=Path, default=[], help='additional C/assembly source tree')
    parser.add_argument('--include-game', action='store_true')
    parser.add_argument('--unattempted', action='store_true')
    parser.add_argument('--minimum', type=float, default=75, help='binary or structural similarity percentage')
    parser.add_argument('--min-words', type=int, default=8)
    parser.add_argument('--limit', type=int, default=30)
    parser.add_argument('--json', action='store_true')
    args = parser.parse_args()
    if not 0 < args.minimum <= 100 or args.min_words < 1 or args.limit < 1:
        parser.error('minimum must be in (0,100]; min-words and limit must be positive')
    require_fresh()
    image = PsxImage((ROOT / 'disc/SLPS_018.23').read_bytes())
    pairs = [(ROOT / 'asm/matchings/syslib', ROOT / 'src/syslib'), *args.reference]
    references = load_references(pairs, image)
    roots = [ROOT / 'src/syslib', *args.source, *(p[1] for p in args.reference)]
    for relative in ('psy-q-decomp/src', 'psyz/decomp/src', 'sotn-decomp/src/main/psxsdk', 'psy-q/3.5/PSYQ/SRC', 'psy-q/3.6/PSYQ/SRC'):
        path = ROOT.parent / relative
        if path.is_dir():
            roots.append(path)
    # Pick up generated matching references automatically when a sibling has
    # been split locally. Source-only checkouts remain useful without a build.
    for sibling in ('psy-q-decomp', 'psyz/decomp', 'sotn-decomp'):
        base = ROOT.parent / sibling
        assembly = base / 'asm/matchings'
        source = base / 'src'
        if assembly.is_dir() and source.is_dir() and (assembly, source) not in pairs:
            try:
                references.extend(load_references([(assembly, source)]))
            except ValueError as error:
                if 'no source-backed' not in str(error):
                    raise
    names = source_index(roots)
    prepared = [(ref, binary_tokens(ref.function)) for ref in references]
    rows = []
    for function in active_functions(sdk=True):
        if not args.include_game and function.unit.parts[:2] != ('src', 'syslib'):
            continue
        if args.unattempted and source_definition((ROOT / function.unit).read_text(), function.name, allow_fallback=True):
            continue
        if image.read(function.address, len(function.data)) != function.data:
            raise ValueError(f'candidate differs from disc image: {function.path}')
        tokens = instruction_tokens(function)
        binary = binary_tokens(function)
        hits = []
        for ref, ref_binary in prepared:
            if len(tokens) < args.min_words:
                continue
            total = len(tokens) + len(ref.tokens)
            if 200 * min(len(tokens), len(ref.tokens)) / total < args.minimum:
                continue
            # Cheap multiset bounds avoid quadratic sequence work on poor leads.
            bounds = [200 * sum((Counter(a) & Counter(b)).values()) / total
                      for a, b in ((tokens, ref.tokens), (binary, ref_binary))]
            if max(bounds) < args.minimum:
                continue
            structure = 100 * ratio(tokens, ref.tokens)
            masked = 100 * ratio(binary, ref_binary)
            if max(structure, masked) < args.minimum:
                continue
            size = len(tokens) * 4
            exact = len(tokens) == len(ref.tokens) and function.data[:size] == ref.function.data[:size]
            hits.append({'name': ref.function.name, 'source': str(ref.source), 'line': ref.line,
                         'kind': 'C', 'asm': str(ref.function.path),
                         'structure_percent': round(structure, 2), 'masked_binary_percent': round(masked, 2),
                         'evidence': 'exact bytes' if exact else 'address-masked equality' if binary == ref_binary else 'similarity',
                         'source_alternatives': names.get(canonical_name(ref.function.name), []),
                         'name_agrees': canonical_name(function.name) == canonical_name(ref.function.name)})
        hits.sort(key=lambda h: (h['evidence'] == 'exact bytes', h['masked_binary_percent'], h['structure_percent']), reverse=True)
        source_hits = names.get(canonical_name(function.name), [])
        if hits or source_hits:
            rows.append({'candidate': function.name, 'unit': str(function.unit), 'words': len(tokens),
                         'matches': hits[:3], 'name_only_sources': source_hits,
                         'ambiguous': len(hits) > 1 and abs(max(hits[0]['masked_binary_percent'], hits[0]['structure_percent']) - max(hits[1]['masked_binary_percent'], hits[1]['structure_percent'])) <= 3})
    rows.sort(key=lambda r: (not bool(r['matches']), -(max(r['matches'][0]['masked_binary_percent'], r['matches'][0]['structure_percent']) if r['matches'] else 0), r['ambiguous'], r['words'], r['candidate']))
    rows = rows[:args.limit]
    cache = {}
    for row in rows:
        related = [source for hit in row['matches'] for source in hit['source_alternatives']]
        for hit in row['matches'] + row['name_only_sources'] + related:
            if hit['source'] not in cache:
                cache[hit['source']] = provenance(Path(hit['source']))
            hit.update(cache[hit['source']])
    if args.json:
        print(json.dumps({'reference_functions': len(references), 'source_roots': list(map(str, roots)), 'candidates': rows}, indent=2))
    else:
        print(f'{len(references)} binary/assembly references; {len(roots)} source trees. Similarity and names are leads, not verified ports.')
        for row in rows:
            print(f"\n{row['candidate']} ({row['words']} words, {row['unit']})" + (' [ambiguous]' if row['ambiguous'] else ''))
            for hit in row['matches']:
                print(f"  {hit['name']}: {hit['evidence']}; binary {hit['masked_binary_percent']:.2f}%, structure {hit['structure_percent']:.2f}%, name agrees: {hit['name_agrees']}")
                print(f"    {hit['source']}:{hit['line']}")
                for source in hit['source_alternatives']:
                    if source['source'] != hit['source']:
                        print(f"    related {source['kind']}: {source['source']}:{source['line']}")
            for hit in row['name_only_sources']:
                print(f"  name only, {hit['kind']}: {hit['source']}:{hit['line']}")
                if hit['url']:
                    print(f"    {hit['url']}")


if __name__ == '__main__':
    try:
        main()
    except (ValueError, OSError) as error:
        raise SystemExit(str(error))
