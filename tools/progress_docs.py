"""Render the verified progress report from source annotations."""

import json
from html import escape
from string import Template

from decomp import active_functions
from progress_attempts import read_attempts
from score_functions import complexity
from update_badges import data_progress

REPOSITORY = "https://github.com/cdlewis/snowboardkidsplus-decomp"


def progress_card(label, matched, total, unit, extra=""):
    percent = 100 * matched / total
    return f"""      <article class="metric">
        <h2>{label}</h2>
        <strong>{percent:.2f}%</strong>
        <meter min="0" max="{total}" value="{matched}" aria-hidden="true"></meter>
        <p>{matched:,} / {total:,} {unit}{extra}</p>
      </article>"""


def function_row(rank, function, attempt):
    address = f"0x{function.address:08X}"
    size = len(function.data)
    unit = function.unit.as_posix()
    source = f"{REPOSITORY}/blob/main/{escape(unit)}#L{attempt.line}"
    score = attempt.score
    if score is None:
        match = '<span class="unscored">Unattempted</span>'
    else:
        percent = 100 * score
        # Preserve the distinction between an exact score and a rounded near match.
        label = "99.99%" if score < 1 and percent >= 99.995 else f"{float(percent):.2f}%"
        if attempt.kind == "object":
            label += " (object)"
        label = f'<a href="{source}">{label}</a>'
        match = (f'<div class="match"><meter min="0" max="1" value="{float(score)}" '
                 f'aria-hidden="true"></meter><span>{label}</span></div>')
    return (f'          <tr data-size="{size}"><td class="rank">{rank}</td>'
            f'<th scope="row"><code>{escape(function.name)}</code>'
            f'<a class="unit" href="{source}">{escape(unit)}</a></th>'
            f'<td class="address"><code>{address}</code></td>'
            f'<td class="size">{size:,}</td><td class="score">{match}</td></tr>')


def generate(root):
    report = json.loads((root / "build/report.json").read_text())
    measures = report["measures"]
    matched_functions = int(measures.get("matched_functions", 0))
    total_functions = int(measures["total_functions"])
    functions = list(active_functions(root, sdk=True))
    if len(functions) != total_functions - matched_functions:
        raise ValueError("The remaining function count disagrees with the verified progress report")

    attempts = read_attempts(root)
    queue = [(function, attempts[function.unit, function.name]) for function in functions]
    queue.sort(key=lambda row: (row[1].score is None, row[1].kind == "object",
                                -(row[1].score or 0), complexity(row[0].metrics()),
                                row[0].address))
    rows = "\n".join(function_row(rank, *row) for rank, row in enumerate(queue, start=1))

    game = next(category["measures"] for category in report["categories"] if category["id"] == "game")
    game_percent = 100 * int(game["matched_code"]) / int(game["total_code"])
    cards = "\n".join((
        progress_card("Code matched", int(measures["matched_code"]), int(measures["total_code"]),
                      "bytes", f" · Game code {game_percent:.2f}%"),
        progress_card("Initialized data", *data_progress(root), "bytes matched"),
        progress_card("Functions matched", matched_functions, total_functions, "functions"),
    ))

    template = Template((root / "tools/progress_report.html").read_text())
    output = template.substitute(repo=REPOSITORY, cards=cards, rows=rows, remaining=len(queue))
    page = root / "docs/index.html"
    if not page.exists() or page.read_text() != output:
        page.write_text(output)
    print(f"Progress page: docs/index.html ({len(queue)} functions remaining)")
