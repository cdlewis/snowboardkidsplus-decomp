---
name: psx-match-graphics
description: Decompile or diagnose PSX graphics functions using SDK macros and GTE inline operations. Use when scratchpad access, GPU packets, ordering tables, graphics-buffer cursors, or GTE instructions indicate graphics work in Snowboard Kids Plus.
---

# Match graphics code

Use alongside [function matching](../psx-match-function/SKILL.md), which owns
the build, attempt, stopping, and integration workflow.

## Recognize and audit the whole graphics path

Access to the `scratchpad` symbol is a strong routing signal, including through
casts, typed views, aliases, or `getScratchAddr`. Also look for GPU primitive
types, packet tags, ordering-table links, packet-buffer cursors, texture-page or
palette packing, `Gs*` sorting, and COP2/GTE instructions. Scratchpad use alone
does not establish that every operation is graphical; establish its consumers.

Before shaping registers, audit the complete function rather than waiting for
individual field assignments to become diff problems. Read the relevant entries
in [the SDK source map](references/sdk-macros.md), then inspect their actual
definitions and callers. Optional read-only triage:

```sh
python3 .agents/skills/psx-match-graphics/scripts/audit_graphics.py src/path/unit.c
```

The helper follows simple aliases and flags likely macro operations, including
operations already written as macros. It is a discovery aid, not proof of macro
identity or an exhaustive analysis. Inspect indirect aliases and the surrounding
function yourself. Pass multiple files or directories, use `--json` for structured
findings, and add `--root NAME` for a known view the scanner cannot trace. It does
not expand includes or perform control-flow analysis; names and derived values
can produce false positives. Its line ranges identify statements to audit, not
instructions to replace automatically.

## Rebuttable presumption: graphics operations came from macros

**For every line involving scratchpad storage, directly or through an alias,
start with the presumption that the operation uses an SDK macro, SDK inline, or
SDK API. Apply the same presumption to every packet operation. Use the matching
standard abstraction unless affirmative evidence rebuts it.** This includes
initialization, color, coordinates, texture coordinates, vectors, flags, tags,
links, projections, and buffer advancement—not just primitive opcodes.

Trace the operation to its meaning and search for the highest-level appropriate
macro first. Prefer a primitive initializer over composing `setcode` and tag
writes; inspect coordinate/color/UV group setters rather than translating each
store independently. For GTE sequences, inspect composite and leaf inline macros
before writing arithmetic or assembly. Preserve real library calls when call
targets and ABI establish them; uppercase APIs and lowercase macros can differ.

An SDK macro missing from `include/psx/` is a header gap, not evidence of
handwritten code. Port the relevant definition into its canonical project header
with source/version attribution and compatible types. Preserve argument
evaluation, widths, masks, clobbers, and hazards. Do not silently replace an SDK
macro's semantics or import an entire foreign header.

### What rebuts the presumption

Retain handwritten C only after documenting one of these:

- Original source or attributable reference material identifies the operation as
  handwritten and agrees with the target's behavior and layout.
- A documented SDK/reference search finds no applicable abstraction, and the
  operation's semantics establish ordinary application logic rather than an
  expanded SDK operation.
- Reproducible comparisons of applicable macro forms against the target
  distinguish the required operation: name the header/version and definitions,
  archive the compiled attempts, and identify the specific semantic or
  instruction difference. Check relevant grouped, packed, and inline variants
  before concluding that a scalar spelling is required.

One worse score, a missing local definition, unfamiliar assembly, or a guessed
store-width pattern is not proof. Distinguish final memory state from access
width/order, tag preservation, side effects, and compiler scheduling. Reject
only the forms actually tested. Binary equivalence alone cannot prove the
historical source spelling; state that uncertainty without abandoning the
presumption.

Make an exception only for the demonstrated operation; keep SDK abstractions on
the rest of the path. An exception does not authorize authored assembly. A new
project helper needs a meaningful reusable operation and an explicit non-SDK
identity; do not invent a macro merely to conceal raw stores, dummy side effects,
or a match-forcing sequence.

## Preserve storage, ABI, and ownership

Keep canonical hardware symbols, their linker bindings, and extern declarations
intact. Do not redefine a symbol as a numeric-address macro, remove it, or ban
its address range to improve matching. SDK fixed-hardware accessors are distinct
from linker symbols: inspect both code generation and the user's required
representation before changing existing access sites. Avoid unrelated sweeping
scratchpad conversions.

Recover typed scratchpad views from all consumers; a shared base can hold
different layouts at different times. Use SDK primitive/vector/matrix types,
typed fields and array strides, not fabricated layouts or raw-offset aliases.
Distinguish a DMA link from contiguous buffer advancement; do not substitute
`nextPrim` for pointer traversal. Measure candidate cursor forms rather than
inferring one from a reference project's style.

Compare preprocessed macro expansions and candidate disassembly before changing
source shape. Preserve SDK inline constraints and delay requirements; use
established inline definitions rather than bespoke assembly. Validate changed
headers against every consumer through the full-image gate. Migrate owned data
with [data matching](../psx-match-data/SKILL.md); an isolated function match does
not establish packet/table storage or integration correctness.
