const controls = document.querySelector(".controls");
const search = document.querySelector("#search");
const sort = document.querySelector("#sort");
const count = document.querySelector("#result-count");
const noResults = document.querySelector("#no-results");
const body = document.querySelector("#function-rows");
const rows = Array.from(body.rows);
const searchText = new Map(rows.map(row => {
    const fields = row.querySelectorAll("th code, .unit, .address");
    return [row, Array.from(fields, field => field.textContent.toLowerCase()).join(" ")];
}));

function filterRows() {
    const query = search.value.trim().toLowerCase();
    let visible = 0;
    for (const row of rows) {
        row.hidden = !searchText.get(row).includes(query);
        visible += Number(!row.hidden);
    }
    count.textContent = query ? `${visible} of ${rows.length} functions` : `${rows.length} functions`;
    noResults.hidden = !query || visible !== 0;
}

function sortRows() {
    const ordered = sort.value === "size"
        ? [...rows].sort((a, b) => Number(b.dataset.size) - Number(a.dataset.size))
        : rows;
    body.append(...ordered);
}

search.addEventListener("input", filterRows);
sort.addEventListener("change", sortRows);
controls.hidden = false;
filterRows();
sortRows();
