# Matching learnings

PSX matching techniques. Build commands are documented in [README.md](README.md#building).

## Start from evidence

- Use the active `INCLUDE_ASM` and splat's original bytes as the function
  oracle. Splitting or renaming leaves stale assembly on disk; directory scans
  alone can rank functions that are already matched.
- Use reference C to identify algorithms, names, ownership, and likely types.
  Verify layouts, APIs, integer widths, signedness, and scheduling against
  the original assembly.
- Rank simple candidates with `tools/score_functions.py`. Its score measures
  instruction count, control flow, calls, stack, and undecoded words. It is a
  transparent search aid, not a learned probability or a match percentage.
- `tools/find_similar_functions.py` compares normalized MIPS instructions.
  Its structural similarity can find related routines across split files and
  explicit reference asm directories. It does not establish equivalent behavior.

## Keep the experiment reproducible

- Begin with a verified full build. Keep attempts in ignored `nonmatchings/`,
  using actual project headers and canonical declarations.
- `tools/match_function.py` compiles a single-body attempt through the real
  build pipeline, with the owning unit's flags. It resolves externals against
  the verified ELF and compares linked bytes at the original address.
- It records source and preprocessed hashes, target hash, compiler path,
  assembler version, and flags. Outputs include disassembly, diff, and JSON.
  A scratch match must still pass integration and the full-image gate.
- The isolated helper accepts extern-only data references. Owned data, local
  static data, and jump tables need a full translation-unit build and objdiff.
- SDK units may require `-fno-builtin` to retain an actual `memcpy` call.
  Otherwise a constant-size copy can become an inline loop despite correct C.
  Preserve verified options in the unit's `cc1_flags` setting; isolated reports
  record those options too.
- An isolated function's ELF size can exclude padding that splat includes
  in its oracle extent. `sin_1` has a four-byte tail and `SpuSetNoiseClock`
  has an eight-byte tail: exact instruction bodies can therefore score below
  100%. Compare the complete linked unit, including original section padding,
  then require the full-image gate; do not discard the padding to claim a match.
- Refresh captures after changing a header, flags, compiler, or unit boundary.
  A permuter winner from an old prelude is not proof for today's source.

## Fix structure before register allocation

- Check argument count/types, return type, loads/stores, loop bounds, signed
  comparisons, calls, and branches before tuning variable order.
- Make one source-shape change per experiment. Try a named intermediate,
  expression grouping, loop form, or assignment order when the remaining
  difference is allocation or scheduling. Keep the C meaningful.
- For the SDK trig routines, early returns and explicit branch arms matter.
  GCC 2.7.2 reproduced `sin_1` after restoring its final `if/else` order and
  `rcos` after replacing the shared result local with early returns. The same
  compiler and assembler settings reproduce both complete units; a blanket
  compiler-version change was unnecessary.
- Separate compiler output differences from assembler scheduling. In the
  remaining `memmove` attempt, cc1 itself emits a direct return instead of
  the original shared-epilogue jump. In `SsUtSetReverbFeedback`, cc1 reuses
  the mask constant 16 for variable-count shifts where the original uses
  immediate shifts. Changing section padding cannot fix either body difference.
- Check indexed-load macro expansion before changing C or the compiler.
  `StopRCnt` and both voice-volume setters had matching compiler structure,
  but ASPSX-style expansion used `$at` where the executable used the load
  destination. The real ASPSX 2.77 executable confirmed that the emulator's
  default was correct. Delegating these macros to GNU as reproduced the
  complete counter and voice-volume modules with one GCC 2.7.2 configuration
  and no function-specific overrides. Compare whole modules: switching to
  GCC 2.8.x made already-matched functions differ.
- The same indexed-load setting reproduces the volume/expression, damper,
  reverb-depth and RPN sequence controllers, program changes, and VAB
  close/transfer objects without changing their C logic. Apply it per unit
  only after comparing the entire object; other partial modules retain
  genuine control-flow and register-allocation differences.
- With those assembler differences removed, small source-shape residuals
  become visible: an `int` local for the current channel reproduces the
  base/index operand order in `_SsContResetAll`, and the original success
  branch in `SsUtGetProgAtr` reproduces its return layout. Neither requires
  register pinning, inline instructions, or compiler changes.
- Audit real compiled C before preserving a compiler override: both controller
  units reproduce with the standard GCC 2.7.2 configuration, making their
  earlier per-function compiler and scheduling selections unnecessary.
  In `PadInfoAct`, a five-case `switch` reproduces the original dispatch and
  its owned jump table; an `if` chain cannot establish that match. Preserve
  the table's address and trailing alignment bytes when carving it into C.
- Cross-check SDK type comments against actual copy lengths and member access.
  This executable copies 0x5C bytes for `DRAWENV`; a reference header's 0x58
  comment is insufficient evidence for its size.
- A cast inside a zero test may disappear because signedness cannot change
  whether the value is zero. The ADSR builder's `lh` for `release_mode` required
  correcting the shared field to `s16`; casting the existing `u16` field still
  emitted `lhu`. Verify every consumer after correcting the declaration.
- Prefer typed canonical structures/unions over invented byte offsets or
  `void *` arithmetic. Infer layouts from multiple consumers, then verify
  target `sizeof` and offsets using the PSX compiler; host layouts differ.
- A shared header affects every consumer. Rebuild and check those consumers,
  not just the function whose diff improved.
- Do not freeze a near match with inline assembly, register variables, dummy
  side effects, or nonsensical operators. Permuter output needs human review.
- A relocation's printed symbol can differ while its final address and linked
  bytes agree. Check the linked result before changing correct C to improve
  an object-level fuzzy score.

## Match data as carefully as code

- Represent fixed BIOS ABI locations with named pointer macros in the owning
  header. For example,
  `BIOS_DCB_BASE` and `BIOS_DCB_BYTES` access the BIOS-owned variables at
  `0x150` and `0x154`; their storage is supplied by the BIOS. This preserves
  the original zero-base loads with the normal assembler pipeline.
- Preserve ownership, section, address, size, order, and alignment together.
  Inspect the object and linked map instead of inferring placement from C.
- Explicit zero initialization can land in `.data`, `.sdata`, or BSS depending
  on compiler settings. Small-data declarations also affect `$gp` addressing
  and load hazards. Keep `-G` consistent through cc1, maspsx, and GNU as.
- Use `tools/data_diff.py SYMBOL` to compare a sized symbol at its expected
  address. It checks freshness and rejects identical bytes moved elsewhere.
  Supply `--size` when neither config nor ELF has reliable size metadata.
- Use `--find-first-mismatch` to locate executable differences, including
  header changes and missing/extra trailing bytes. Use `./tools/build-and-verify.sh` as the gate.
- BSS has no original file bytes. Establish its address, allocation size,
  alignment, and references separately; a file-byte tool cannot prove it.

## PSX compiler and layout

- PSX is little endian: the low byte of a word is at offset zero.
- This project uses GCC PSX, maspsx, and R3000 load/branch delays.
- All game C uses one compiler version. SDK objects may have another origin.
  Verify suspected flags with multiple functions and the whole image; one
  lucky leaf match does not identify the original build configuration.
- Function-scoped scheduling overrides are a documented build workaround,
  not evidence that a logical module was originally split into many files.
