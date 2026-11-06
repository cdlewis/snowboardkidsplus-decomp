"""Translate GCC assembly to reproduce PSY-Q assembler behavior."""

import re
from typing import List

from .instructions import (
    strip_comments,
    line_loads_from_reg,
    uses_at,
    parse_load_or_store,
    div_needs_expanding,
    expand_load_immediate,
    expand_move,
    is_label,
    is_instruction,
    expand_macro,
    load_immediate_single,
    load_immediate_double,
    branch_mnemonics,
    jump_mnemonics,
    load_mnemonics,
    store_mnemonics,
)


class MaspsxProcessor:
    is_reorder = True
    skip_instructions = 0
    file_num = 1
    line_index = 0

    def __init__(
        self,
        lines: List[str],
        sdata_limit=0,
        expand_div=False,
        expand_li=False,
        nop_at_expansion=False,
        nop_mflo_mfhi=True,
        sltu_at=False,
        addiu_at=False,
        div_uses_tge=False,
        gp_allow_offset=False,
        gp_allow_la=False,
        use_comm_section=False,
        use_comm_for_lcomm=False,
        gas_reorder=False,
        gas_indexed_loads=False,
    ):
        self.lines = [x.strip() for x in lines]
        self.use_gas_reorder = gas_reorder
        self.gas_indexed_loads = gas_indexed_loads

        self.sdata_limit = sdata_limit

        self.expand_div = expand_div
        self.expand_li = expand_li

        self.nop_at_expansion = nop_at_expansion
        self.nop_mflo_mfhi = nop_mflo_mfhi

        self.sltu_at = sltu_at
        self.addiu_at = addiu_at
        self.div_uses_tge = div_uses_tge

        self.gp_allow_offset = gp_allow_offset
        self.gp_allow_la = gp_allow_la

        self.use_comm_section = use_comm_section
        self.use_comm_for_lcomm = use_comm_for_lcomm

        self.bss_entries: dict[str, int] = {}
        self.sbss_entries: dict[str, int] = {}
        self.sdata_entries: dict[str, int] = {}

        self.comm_symbols: set[str] = set()

    def preprocess_lines(self) -> None:
        in_sdata = False
        uses_size = False

        for line in self.lines:
            if line == "":
                continue

            if line.startswith(".align"):
                # TODO: worry about alignment later
                continue

            if line.startswith(".globl"):
                continue

            if line.startswith(".text"):
                in_sdata = False
                continue
            if line.startswith(".data"):
                in_sdata = False
                continue
            if line.startswith(".rdata"):
                in_sdata = False
                continue

            if line.startswith(".section"):
                # w26-a3: generic ELF-style `.section NAME[,"FLAGS",@TYPE]` directive,
                # not just the bare `.section .text` this used to special-case. CC1PSX
                # emits this form (via GCC's generic named_section() output path,
                # confirmed present as literal `.section %s,"aw",@progbits` /
                # `"a",@progbits` / `"ax",@progbits" format strings inside CC1PSX.EXE)
                # for any global carrying an explicit
                # `__attribute__((section(".data"/".bss"/...)))` -- used throughout this
                # tree (§3.12 "force absolute placement" lever) to pin a would-be-.sdata/
                # .sbss global into regular .data/.bss so it addresses absolute instead
                # of gp-relative, matching the retail oracle. The narrow `endswith(".text")`
                # check here only recognised `.section .text`; any OTHER named section
                # (`.section .data,"aw",@progbits`, `.section .bss,"aw",@progbits`, ...)
                # fell through every branch below and -- if a prior bare `.sdata` block
                # had left `in_sdata` True with no intervening `.text`/`.data`/`.rdata` --
                # hit the catch-all "Unable to parse .sdata instruction" exception in the
                # `in_sdata` scan below. (Real trigger, libgpu/SYS.cpp: a `.sdata`-placed
                # const table's literal-pool tail is immediately followed by
                # `GEnv_drv`'s forced `.section .data,"aw",@progbits`, no `.text` between.)
                # Fix: parse the section NAME out of any `.section` directive and treat it
                # exactly like the matching bare directive would -- `sdata`/`sbss` (re-)enter
                # the sdata scan (parity with bare `.sdata`, in case a future TU ever forces
                # `__attribute__((section(".sdata")))`), anything else (`.text`, `.data`,
                # `.bss`, `.rodata`/`.rdata`, or an arbitrary named section) resets it, same
                # as the existing bare `.text`/`.data`/`.rdata` handling above.
                m = re.match(r"^\.section\s+\.?([A-Za-z0-9_.]+)", line)
                name = m.group(1) if m else ""
                in_sdata = name in ("sdata", "sbss")
                continue

            if line.startswith("#"):
                continue

            if line.startswith(".sdata"):
                in_sdata = True
                continue

            if line.startswith(".file"):
                in_sdata = False
                continue

            if line.startswith(".extern"):
                in_sdata = False
                # GCC records the size of external small-data objects here.
                # Account for their single-instruction $gp accesses when
                # deciding whether a dependent load needs a delay-slot nop.
                match = re.match(r"\.extern\s+([^,]+),\s*(\d+)$", line)
                if match:
                    size = int(match.group(2))
                    if 0 < size <= self.sdata_limit:
                        self.sdata_entries[match.group(1)] = size
                continue

            if line.startswith(".comm") or line.startswith(".lcomm"):
                # e.g.	.comm	MENU_RadarScale_800AB480,4
                in_sdata = False
                _, var = line.split()
                symbol, size_str = var.split(",")
                size = int(size_str)
                if size <= self.sdata_limit:
                    self.sbss_entries[symbol] = size
                else:
                    self.bss_entries[symbol] = size

                if line.startswith(".comm"):
                    self.comm_symbols.add(symbol)
                continue

            if in_sdata:
                # NOTE: newer compilers emit .size for sdata, old ones do not...
                if match := re.match(r"\.size\s+([^,]+),([0-9]+)", line):
                    current_symbol = match.group(1)
                    size = int(match.group(2))
                    self.sdata_entries[current_symbol] = size
                    uses_size = True
                    continue

                if not uses_size:
                    if line.endswith(":"):
                        current_symbol = line.replace(":", "")
                        self.sdata_entries[current_symbol] = 0
                    else:
                        if line.startswith(".type"):
                            continue

                        if line.startswith(".space"):
                            _, size_str = line.split()
                            size = int(size_str)
                        elif line.startswith(".word"):
                            size = 4
                        elif line.startswith(".half") or line.startswith(".short"):
                            size = 2
                        elif line.startswith(".byte"):
                            size = 1
                        elif line.startswith(".ascii"):
                            # e.g. .ascii	"Map poly groups\000"
                            # NOTE: len('.ascii\t""') == 9
                            size = len(line) - 9
                        else:
                            raise Exception(
                                f"Unable to parse .sdata instruction: {line}"
                            )
                        self.sdata_entries[current_symbol] += size

    def process_lines(self):
        self.is_reorder = True
        self.gas_reorder_passthrough = False
        self.skip_instructions = 0
        self.file_num = 1

        self.bss_entries = {}
        self.sbss_entries = {}
        self.sdata_entries = {}

        self.preprocess_lines()

        res = []
        in_include_asm_hack = False
        for i, line in enumerate(self.lines):
            self.line_index = i

            if ".ent\t__maspsx_include_asm_hack" in line:
                in_include_asm_hack = True

            if in_include_asm_hack:
                if "# maspsx-keep" in line:
                    res += [line]
                else:
                    res += [f"# {line} # DEBUG: skipped due to include asm hack"]
                if ".end\t__maspsx_include_asm_hack" in line:
                    in_include_asm_hack = False
                continue

            if is_instruction(line) and self.skip_instructions > 0:
                self.skip_instructions -= 1
                res += [f"# {line}  # DEBUG: skipped"]
            else:
                res += self.process_line(line)

        for section, entries in [
            ("sbss", self.sbss_entries),
            ("bss", self.bss_entries),
        ]:
            for i, (symbol, size) in enumerate(entries.items()):
                if i == 0:
                    res.append(f".section .{section}")

                if self.use_comm_section and (
                    symbol in self.comm_symbols or self.use_comm_for_lcomm
                ):
                    # implicit alignment for COMMON
                    res.append(f"\t.comm {symbol},{size}")
                    continue

                if section == "sbss":
                    if size >= 8:
                        res.append("\t.align 3")
                    elif size >= 4:
                        res.append("\t.align 2")
                    elif size >= 2:
                        res.append("\t.align 1")

                # only mark bss symbols as global -- and only those that
                # came from `.comm` (a true tentative definition).  A
                # `.lcomm` is a FILE STATIC: GNU as keeps it local and so
                # must we.  Without this guard maspsx globalizes every
                # file-static that lands in .bss (24 tree-wide; the five
                # `static char gSwapFileName[..]` copies then collide with
                # the one genuinely global symbol of that name).
                # w62-a18 SYMBOL_LEDGER 1.2; probe w63a20/lcomm_probe.py.
                if section == "bss" and symbol in self.comm_symbols:
                    res.append(
                        f"\t.globl {symbol}",
                    )
                res.extend(
                    [
                        f"{symbol}:",
                        f"\t.space {size}",
                    ]
                )

        return res

    def get_next_instruction(
        self, skip=0, ignore_nop=False, ignore_set=False, ignore_label=False
    ):
        i = self.line_index + 1
        while i < len(self.lines):
            line = self.lines[i]
            if is_instruction(
                line,
                ignore_nop=ignore_nop,
                ignore_set=ignore_set,
                ignore_label=ignore_label,
            ):
                if skip == 0:
                    return line
                skip -= 1
            i += 1

        return ""  # warn user?

    def _uses_gp(self, line: str) -> bool:
        if self.sdata_limit == 0:
            return False

        line = strip_comments(line)
        if uses_at(line):
            op, *rest = line.split("\t")
            if op in load_mnemonics or op in store_mnemonics:
                (
                    _,
                    _,
                    operand,
                    _,
                    _,
                ) = parse_load_or_store(" ".join(rest))

                if operand.count("+") == 1:
                    symbol, _ = operand.split("+")
                    gp_allowed = self.gp_allow_offset or symbol not in self.comm_symbols
                else:
                    symbol = operand
                    gp_allowed = True

                if gp_allowed and (
                    symbol in self.sbss_entries or symbol in self.sdata_entries
                ):
                    return True

        return False

    def _handle_nop_before_next_instruction(
        self, next_instruction: str, r_dest: str
    ) -> List[str]:
        res: List[str] = []

        if line_loads_from_reg(next_instruction, r_dest):
            nop_required = False

            if not uses_at(next_instruction):
                reason = f"'{next_instruction}' does not use $at"
                nop_required = True
            if self._uses_gp(next_instruction):
                reason = f"'{next_instruction}' uses $gp"
                nop_required = True
            if uses_at(next_instruction) and self.nop_at_expansion:
                reason = (
                    f"'{next_instruction}' inject nop beween {r_dest} and $at expansion"
                )
                nop_required = True

            if nop_required:
                label = self.get_next_instruction(
                    skip=0, ignore_nop=True, ignore_set=True
                )
                nop = f"nop # DEBUG: Reuse of '{r_dest}'. {reason}"
                if is_label(label):
                    self.skip_instructions = 1
                    res.append(label)
                res.append(nop)
        else:
            res.append(
                f"#nop # DEBUG: '{next_instruction}' does not load from {r_dest}"
            )

        return res

    def _handle_mflo_mfhi(self, r_source=None) -> List[str]:
        # we cannot use a div/mult within 2 instructions of mflo/mfhi
        res: List[str] = []

        if not self.nop_mflo_mfhi:
            return res

        next_instruction = self.get_next_instruction(
            skip=0, ignore_nop=True, ignore_set=True, ignore_label=True
        )
        next_next_instruction = self.get_next_instruction(
            skip=1, ignore_nop=True, ignore_set=True, ignore_label=True
        )

        if any(
            next_instruction.startswith(x)
            for x in ["mult\t", "multu\t", "div\t", "divu\t", "rem\t", "remu\t"]
        ):
            # #nop
            # #nop
            # mult...
            skip = 0
            while True:
                inst = self.get_next_instruction(skip=skip)
                skip += 1
                if inst == next_instruction:
                    res.append("nop")
                    res.append("nop")
                    if div_needs_expanding(inst):
                        res.append("# DEBUG: div needs expanding")
                        skip -= 1
                    else:
                        res.append(expand_move(inst))
                    break
                if not inst.startswith("#"):
                    res.append(expand_move(inst))
            self.skip_instructions = skip

        elif any(
            next_next_instruction.startswith(x)
            for x in ["mult\t", "multu\t", "div\t", "divu\t", "rem\t", "remu\t"]
        ):

            # #nop
            # #nop
            # bne or addu or lh ...
            # mult ...
            skip = 0
            no_reorder = False
            while True:
                inst = self.get_next_instruction(skip=skip)

                if inst.startswith(".set") and inst.endswith("noreorder"):
                    no_reorder = True
                    skip += 1
                    continue

                skip += 1
                if inst == next_instruction:
                    op, *_ = inst.strip().split()
                    if op in load_mnemonics:
                        # allow for $at handling later in the script
                        skip = 0
                        break

                    if op in ("mflo", "mfhi"):
                        # allow for mflo/mfhi handling later on
                        skip = 0
                        break

                    if op == "li":
                        expanded = expand_load_immediate(inst)

                        if self.expand_li:
                            res += expanded
                        else:
                            res.append(inst)

                        if len(expanded) == 2:
                            res.append(
                                "#nop  # DEBUG: mflo/mfhi with mult/div/rem and li expands to 2 ops"
                            )
                        else:
                            res.append(
                                "nop  # DEBUG: mflo/mfhi with mult/div/rem and li expands to 1 op"
                            )

                    else:

                        if no_reorder:
                            res.append(
                                "nop  # DEBUG: mflo/mfhi with mult/div/rem and 1 instruction (noreorder)"
                            )
                            res.append(".set\tnoreorder")
                            res.append(expand_move(inst))
                        else:
                            if r_source and line_loads_from_reg(inst, r_source):
                                # NOTE: only relevant when div has been expanded (i.e. -0 flag)
                                res.extend(
                                    [
                                        f"nop  # DEBUG: mflo/mfhi with mult/div/rem and 1 instruction which loads from {r_source}",
                                        expand_move(inst),
                                    ]
                                )
                            else:
                                if op in branch_mnemonics:
                                    res.extend(
                                        [
                                            inst,
                                            "nop # DEBUG: mflo/mfhi with mult/div/rem and 1 instruction (branch)",
                                        ]
                                    )
                                else:
                                    maybe_label = self.get_next_instruction(skip=skip)
                                    if is_label(maybe_label):
                                        res.extend(
                                            [
                                                expand_move(inst),
                                                maybe_label,
                                                "nop  # DEBUG: mflo/mfhi with mult/div/rem and 1 instruction (label)",
                                            ]
                                        )
                                        skip += 1
                                    else:
                                        res.extend(
                                            [
                                                expand_move(inst),
                                                "nop  # DEBUG: mflo/mfhi with mult/div/rem and 1 instruction",
                                            ]
                                        )

                elif inst == next_next_instruction:
                    # reached mult/div/rem
                    if div_needs_expanding(inst):
                        res.append("# DEBUG: div needs expanding")
                        skip -= 1
                    else:
                        res.append(inst)
                    break
                elif not inst.startswith("#"):
                    res.append(expand_move(inst))
            self.skip_instructions = skip

        else:
            # do nothing
            pass

        return res

    def process_line(self, line: str):
        res = []

        if len(line) == 0:
            return [line]

        if line.startswith("#"):
            return []

        if line.startswith("."):
            if (
                line.startswith(".def\t")
                or line.startswith(".begin\t")
                or line.startswith(".bend\t")
            ):
                # skip these coff directives - gnu as does not like them
                pass

            elif line == ".set\tmaspsx_gas_reorder":
                # A raw reorder-mode cc1 region must reach GNU as unchanged.
                # Keep maspsx from materializing branch/jump nops itself while
                # emitting the real directive for the downstream assembler.
                self.is_reorder = False
                self.gas_reorder_passthrough = True
                res.append(".set\treorder")

            elif line.startswith(".set\t"):
                if line.endswith("\tnoreorder"):
                    self.is_reorder = False
                elif line.endswith("\treorder"):
                    self.is_reorder = True
                if self.gas_reorder_passthrough:
                    res.append(line)

            elif line.startswith(".file\t"):
                # fix same-numbered files
                _, file_num, filename = line.split(maxsplit=2)
                res.append(f".file\t{self.file_num} {filename}")
                self.file_num += 1

            elif line.startswith(".ent\t"):
                res.append(line)
                if self.use_gas_reorder:
                    # SDK cc1 2.7.2 leaves macro stores and jumps in reorder
                    # mode. Delegate their scheduling to GNU as -O2; keep
                    # maspsx's pseudo-op normalization (notably move = addu).
                    self.gas_reorder_passthrough = True
                    self.is_reorder = False
                    res.append(".set\treorder")
                else:
                    self.gas_reorder_passthrough = False
                    res.append(".set\tnoreorder")

            elif line.startswith(".comm") or line.startswith(".lcomm"):
                # already handled via preprocess_lines
                pass

            elif line.startswith(".data"):
                res.append(".section .data")
            elif line.startswith(".sdata"):
                res.append(".section .sdata")
            elif line.startswith(".rdata"):
                res.append(".section .rodata")

            else:
                res.append(line)

            return res

        if line.startswith("$L"):
            return [line]

        actual_r_dest = None
        is_macro = ";" in line
        if is_macro:
            expanded = expand_macro(line)
            if len(expanded) > 0:
                actual_op, *actual_rest = expanded[-1]
                if actual_op in load_mnemonics:
                    _, actual_r_dest, _, _, _ = parse_load_or_store(
                        " ".join(actual_rest)
                    )

        op, *rest = line.split()

        if op in load_mnemonics:
            r_source, r_dest, operand, is_addend, needs_expanding = parse_load_or_store(
                " ".join(rest)
            )

            next_instruction = self.get_next_instruction(
                skip=0, ignore_nop=True, ignore_set=True, ignore_label=True
            )
            # Naively handle scenario where *next* line is a macro
            if ";" in next_instruction:
                next_instruction = next_instruction.split(";")[0]

            if not needs_expanding:
                # newer GCCs can emit %hi() and %lo() separately...
                res.append(f"{line} # DEBUG: leaving for assembler to expand")
                extra_nops = self._handle_nop_before_next_instruction(
                    next_instruction, r_dest
                )
                res.extend(extra_nops)

            elif is_addend and r_source is None:
                # e.g. lb	$s0,D_800E52E0
                if operand.count("+") == 1:
                    symbol, offset = operand.split("+")
                    gp_rel = f"%gp_rel({symbol}+{offset})($gp)"
                    gp_allowed = self.gp_allow_offset or symbol not in self.comm_symbols
                else:
                    symbol = operand
                    gp_rel = f"%gp_rel({symbol})($gp)"
                    gp_allowed = True

                if gp_allowed and (
                    symbol in self.sdata_entries or symbol in self.sbss_entries
                ):
                    res.append(f"{op}\t{r_dest},{gp_rel}")
                else:
                    res.append(line)

                extra_nops = self._handle_nop_before_next_instruction(
                    next_instruction, r_dest
                )
                res.extend(extra_nops)

            elif is_addend and r_source:
                # e.g. lw	$2,test_sym($4)
                if self.gas_indexed_loads:
                    # GNU as may use the load destination as its address
                    # temporary. ASPSX uses $at instead; retain that default.
                    res.append(line)
                elif self.addiu_at:
                    res.extend(
                        [
                            "# EXPAND_AT START",
                            ".set\tnoat",
                            f"lui\t$at,%hi({operand})",
                            f"addiu\t$at,$at,%lo({operand})",
                            f"addu\t$at,$at,{r_source}",
                            f"{op}\t{r_dest},0x0($at)",
                            ".set\tat",
                            "# EXPAND_AT END",
                        ]
                    )
                else:
                    res.extend(
                        [
                            "# EXPAND_AT START",
                            ".set\tnoat",
                            f"lui\t$at,%hi({operand})",
                            f"addu\t$at,$at,{r_source}",
                            f"{op}\t{r_dest},%lo({operand})($at)",
                            ".set\tat",
                            "# EXPAND_AT END",
                        ]
                    )

                extra_nops = self._handle_nop_before_next_instruction(
                    next_instruction, r_dest
                )
                res.extend(extra_nops)

            else:
                if r_source and (int(operand) > 32767 or int(operand) < -32768):
                    # e.g. lhu	$2,49344($2)
                    res.extend(
                        [
                            "# EXPAND_AT START",
                            ".set\tnoat",
                            f"lui\t$at,%hi({operand})",
                            f"addu\t$at,{r_source},$at",
                            f"{op}\t{r_dest},%lo({operand})($at)",
                            ".set\tat",
                            "# EXPAND_AT END",
                        ]
                    )
                else:
                    # e.g. lhu	$2,528482304
                    res.append(line)

                # Naively handle scenario where *current* line is a macro
                if actual_r_dest is not None:
                    r_dest = actual_r_dest

                extra_nops = self._handle_nop_before_next_instruction(
                    next_instruction, r_dest
                )
                res.extend(extra_nops)

        elif op in store_mnemonics or (op == "la" and self.sdata_limit > 0):
            r_source, r_dest, operand, is_addend, _ = parse_load_or_store(
                " ".join(rest)
            )

            if is_addend and r_source is None:
                # e.g. sw	$v0,D_800E52E0
                if operand.count("+") == 1:
                    symbol, offset = operand.split("+")
                    gp_rel = f"%gp_rel({symbol}+{offset})($gp)"
                    gp_allowed = self.gp_allow_offset or symbol not in self.comm_symbols
                else:
                    symbol = operand
                    gp_rel = f"%gp_rel({symbol})($gp)"
                    gp_allowed = True

                if op == "la" and not self.gp_allow_la:
                    gp_allowed = False

                if gp_allowed and (
                    symbol in self.sdata_entries or symbol in self.sbss_entries
                ):
                    res.append(f"{op}\t{r_dest},{gp_rel}")
                elif op == "la" and not gp_allowed and (
                    symbol in self.sdata_entries or symbol in self.sbss_entries
                ):
                    # GNU as can relax a bare la to a GP-relative addiu when
                    # -G is enabled. Preserve the ASPSX version's absolute
                    # address choice even for a small-data symbol.
                    res.append(f"lui\t{r_dest},%hi({operand})")
                    res.append(f"addiu\t{r_dest},{r_dest},%lo({operand})")
                else:
                    res.append(line)
            elif is_addend and r_source:
                # e.g. sw	$a0,ctlbuf($v0)
                if self.addiu_at and op != "la":
                    res.extend(
                        [
                            "# EXPAND_AT START",
                            ".set\tnoat",
                            f"lui\t$at,%hi({operand})",
                            f"addiu\t$at,$at,%lo({operand})",
                            f"addu\t$at,$at,{r_source}",
                            f"{op}\t{r_dest},0x0($at)",
                            ".set\tat",
                            "# EXPAND_AT END",
                        ]
                    )
                else:
                    res.append(line)
            elif r_source and (int(operand) > 32767 or int(operand) < -32768):
                # e.g. sw	$2,56200($4)
                res.extend(
                    [
                        "# EXPAND_AT START",
                        ".set\tnoat",
                        f"lui\t$at,%hi({operand})",
                        f"addu\t$at,{r_source},$at",
                        f"{op}\t{r_dest},%lo({operand})($at)",
                        ".set\tat",
                        "# EXPAND_AT END",
                    ]
                )
            else:
                res.append(line)

        elif op in branch_mnemonics or op in jump_mnemonics:
            res.append(line)
            if self.is_reorder and not self.gas_reorder_passthrough:
                res.append("nop  # DEBUG: branch/jump")

        elif op == "move":
            # expand move $2,$16 to addu $2,$16,$zero
            res.append(expand_move(line))

        elif op in ("addu", "subu", "sra", "srl", "srr", "sll", "or"):
            # no extra processing required
            res.append(line)
            # TODO: check if this line is a macro and insert a nop if required...

        elif op == "li":
            # TODO: handle non-soft floats?
            if self.expand_li:
                res += expand_load_immediate(line)
            else:
                res.append(line)

        elif op == "li.s":
            res += load_immediate_single(line)

        elif op == "li.d":
            res += load_immediate_double(line)

        elif op in ("mflo", "mfhi"):
            res.append(line)
            res += self._handle_mflo_mfhi()

        elif op == "break":
            # turn 'break 7' into 'break 0x0,0x7'
            num = int(rest[0], 0)
            line = f"break\t0x{num >> 10:X},0x{num & 0x3FF:X}"
            res.append(line)

        elif op in ("div", "rem"):
            r_dest, r_source, r_operand = rest[0].split(",")
            if r_dest in ("$zero", "$0"):
                # e.g. div $zero, $v0, $a0
                return [line]

            move_from = "mfhi" if op == "rem" else "mflo"
            if self.expand_div:
                res.extend(
                    [
                        "# EXPAND_DIV START",
                        ".set\tnoat",
                        f"div\t$zero,{r_source},{r_operand}",
                        f"bnez\t{r_operand},.L_NOT_DIV_BY_ZERO_{self.line_index}",
                        "nop",
                        "break\t0x7",
                        f".L_NOT_DIV_BY_ZERO_{self.line_index}:",
                        "addiu\t$at,$zero,-1",
                        f"bne\t{r_operand},$at,.L_DIV_BY_POSITIVE_SIGN_{self.line_index}",
                        "lui\t$at,0x8000",
                        f"bne\t{r_source},$at,.L_DIV_BY_POSITIVE_SIGN_{self.line_index}",
                        "nop",
                        "tge\t$zero,$zero,93" if self.div_uses_tge else "break\t0x6",
                        f".L_DIV_BY_POSITIVE_SIGN_{self.line_index}:",
                        f"{move_from}\t{r_dest}",
                        ".set\tat",
                        "# EXPAND_DIV END",
                    ]
                )
            else:
                res.extend(
                    [
                        "# EXPAND_ZERO_DIV START",
                        f"div\t$zero,{r_source},{r_operand}",
                        f"{move_from}\t{r_dest}",
                        "# EXPAND_ZERO_DIV END",
                    ]
                )

            extra_nops = self._handle_mflo_mfhi(r_source=r_dest)
            if len(extra_nops) > 0:
                res += extra_nops
            else:
                next_instruction = self.get_next_instruction(
                    skip=0, ignore_set=True, ignore_label=True
                )
                extra_nops = self._handle_nop_before_next_instruction(
                    next_instruction, r_dest
                )
                res.extend(extra_nops)

        elif op in ("divu", "remu"):
            r_dest, r_source, r_operand = rest[0].split(",")
            if r_dest in ("$zero", "$0"):
                # e.g. divu $zero, $v1, $a2
                return [line]

            move_from = "mfhi" if op == "remu" else "mflo"
            if self.expand_div:
                res.extend(
                    [
                        "# EXPAND_DIVU START",
                        ".set\tnoat",
                        f"divu\t$zero,{r_source},{r_operand}",
                        f"bnez\t{r_operand},.L_NOT_DIV_BY_ZERO_{self.line_index}",
                        "nop",
                        "break\t0x7",
                        f".L_NOT_DIV_BY_ZERO_{self.line_index}:",
                        f"{move_from}\t{r_dest}",
                        ".set\tat",
                        "# EXPAND_DIVU END",
                    ]
                )
            else:
                res.extend(
                    [
                        "# EXPAND_ZERO_DIVU START",
                        f"divu\t$zero,{r_source},{r_operand}",
                        f"{move_from}\t{r_dest}",
                        "# EXPAND_ZERO_DIVU END",
                    ]
                )

            extra_nops = self._handle_mflo_mfhi(r_source=r_dest)
            if len(extra_nops) > 0:
                res += extra_nops
            else:
                next_instruction = self.get_next_instruction(
                    skip=0, ignore_set=True, ignore_label=True
                )
                extra_nops = self._handle_nop_before_next_instruction(
                    next_instruction, r_dest
                )
                res.extend(extra_nops)

        elif op == "sltu":
            r_dest, r_source, r_operand = rest[0].split(",")
            if re.match(r"^-?\d+$", r_operand) or re.match(
                r"^-?0x[A-Fa-f0-9]+$", r_operand
            ):
                value = int(r_operand)
                if self.sltu_at and value < 0:
                    res.append(f"li\t$at,{r_operand}")
                    res.append(f"{op}\t{r_dest},{r_source},$at")
                else:
                    # TODO: do we want to expand sltu into sltiu?
                    res.append(line)
            else:
                res.append(line)

        else:
            res.append(line)

        return res
