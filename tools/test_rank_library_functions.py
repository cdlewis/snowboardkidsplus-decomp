"""Regression tests for source-backed SDK similarity ranking."""

from contextlib import redirect_stdout
from io import StringIO
import json
from pathlib import Path
import tempfile
import unittest

from decomp import parse_asm
from rank_library_functions import (instruction_tokens, load_references,
                                    print_ranking, rank_functions, source_definition)

BODY = [
    (0x24040001, "addiu $a0, $zero, 1"),
    (0x0C000100, "jal helper"),
    (0, "nop"),
    (0x03E00008, "jr $ra"),
    (0, "nop"),
]


def assembly(path, name, body, padding=0):
    path.parent.mkdir(parents=True, exist_ok=True)
    lines = [f"glabel {name}"]
    for index, (word, instruction) in enumerate(body):
        encoded = word.to_bytes(4, "little").hex().upper()
        lines.append(f"/* {0x800 + index * 4:X} {0x80010000 + index * 4:08X} "
                     f"{encoded} */ {instruction}")
    lines.append(f"endlabel {name}")
    for index in range(len(body), len(body) + padding):
        lines.append(f"/* {0x800 + index * 4:X} {0x80010000 + index * 4:08X} 00000000 */ nop")
    path.write_text("\n".join(lines) + "\n")
    return parse_asm(path, Path("src/syslib/candidate.c"))


class LibraryRankingTests(unittest.TestCase):
    def setUp(self):
        self.directory = tempfile.TemporaryDirectory()
        self.addCleanup(self.directory.cleanup)
        self.root = Path(self.directory.name)
        self.asm = self.root / "matching"
        self.source = self.root / "src"
        self.asm.mkdir()
        self.source.mkdir()

    def reference(self, name, body=BODY, padding=0):
        assembly(self.asm / name / (name + ".s"), name, body, padding)
        source = self.source / (name + ".c")
        source.write_text(f"/* reference */\nint {name}(void) {{ return 1; }}\n")
        return source

    def candidate(self, name="candidate", body=BODY, padding=0):
        return assembly(self.root / "candidates" / (name + ".s"), name, body, padding)

    def test_relocations_and_section_padding_are_not_byte_match_claims(self):
        self.reference("sdk_api", padding=3)
        candidate_body = list(BODY)
        candidate_body[1] = (0x0C000200, "jal another_helper")
        candidate = self.candidate(body=candidate_body)
        references = load_references([(self.asm, self.source)])
        self.assertEqual(len(instruction_tokens(references[0].function)), len(BODY))
        self.assertNotEqual(candidate.data, references[0].function.data)
        rows = rank_functions([candidate], references, min_words=1)
        self.assertEqual(rows[0]["similarity_percent"], 100)
        self.assertEqual(rows[0]["matching_instructions"], len(BODY))
        self.assertEqual(rows[0]["reference_line"], 2)
        self.assertNotIn("exact", rows[0])

    def test_rank_order_and_tied_alternatives(self):
        self.reference("api_a")
        self.reference("api_b")
        references = load_references([(self.asm, self.source)])
        different = list(BODY)
        different[0] = (0x24040002, "addiu $a0, $zero, 2")
        rows = rank_functions([self.candidate("near", different),
                               self.candidate("closest")], references, min_words=1)
        self.assertEqual([row["name"] for row in rows], ["closest", "near"])
        self.assertEqual([row["rank"] for row in rows], [1, 2])
        self.assertTrue(rows[0]["ambiguous"])
        self.assertEqual(rows[0]["margin_percent"], 0)
        self.assertEqual(rows[0]["alternatives"][0]["name"], "api_b")
        output = StringIO()
        with redirect_stdout(output):
            print_ranking(rows, as_json=True, limit=1)
        self.assertEqual(len(json.loads(output.getvalue())), 1)

    def test_stale_matching_file_and_guarded_partial_c_are_excluded(self):
        source = self.reference("sdk_api")
        source.write_text(
            '#ifdef NON_MATCHING\nint sdk_api(void) { return 1; }\n'
            '#else\nINCLUDE_ASM("asm/nonmatchings/api", sdk_api);\n#endif\n')
        with self.assertRaisesRegex(ValueError, "no source-backed"):
            load_references([(self.asm, self.source)])
        self.assertIsNone(source_definition(
            'const char *s = "sdk_api(void) { return 1; }";\n'
            'int sdk_api(void);', "sdk_api"))

    def test_calls_are_not_mistaken_for_source_definitions(self):
        self.assertIsNone(source_definition(
            "void other(void) {\n    if (sdk_api(1)) { return; }\n}\n", "sdk_api"))
        self.assertEqual(source_definition(
            "static unsigned long *\nsdk_api(int value) { return 0; }\n", "sdk_api"), 1)

    def test_short_stubs_and_shared_prologues_do_not_inflate_scores(self):
        self.reference("sdk_api")
        references = load_references([(self.asm, self.source)])
        self.assertEqual(rank_functions([self.candidate()], references, min_words=8), [])
        long_body = BODY[:1] + [(0, "nop")] * 40 + BODY[-2:]
        self.assertEqual(rank_functions([self.candidate("long", long_body)],
                                        references, min_words=1), [])

    def test_references_are_deduplicated_and_never_compare_to_themselves(self):
        self.reference("sdk_api")
        pair = (self.asm, self.source)
        references = load_references([pair, pair])
        self.assertEqual(len(references), 1)
        self.assertEqual(rank_functions([references[0].function],
                                        references, min_words=1), [])


if __name__ == "__main__":
    unittest.main()
