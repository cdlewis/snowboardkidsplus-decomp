"""Instruction parsing, register-use checks, and pseudo-op expansion."""

import struct
import re

from typing import List

branch_mnemonics = {
    "beq",
    "bgez",
    "bgtz",
    "blez",
    "bltz",
    "bne",
}
jump_mnemonics = {
    "j",
    "jal",
}
load_mnemonics = {
    "lb",
    "lbu",
    "lh",
    "lhu",
    "lw",
    "lwl",
    "lwr",
}
store_mnemonics = {
    "sb",
    "sh",
    "sw",
    "swl",
    "swr",
    "swc2",
}

single_reg_loads = {
    "mult",
    "multu",
    "div",
    "divu",
    "rem",
    "remu",
    "move",
    "negu",
    "nor",
}
double_reg_loads = {
    "and",
    "andi",
    "or",
    "ori",
    "xor",
    "xori",
    "addu",
    "subu",
    "sll",
    "slr",
    "srl",
    "sra",
    "slt",
    "slti",
    "sltu",
}


def strip_comments(line: str) -> str:
    if line.count("#") > 0:
        line = line.split("#")[0]
    return line.strip()


def line_loads_from_reg(line: str, r_source: str) -> bool:
    """
    NOTE: Returns True even if line might use $at expansion
    """
    line = strip_comments(line)

    # escape dollar
    r_source = r_source.replace("$", r"\$")

    if match := re.match(r"^([A-z][A-z0-9]*)\s+(.*)$", line):
        op, rest = match.group(1, 2)
    else:
        return False

    if op in load_mnemonics:
        # lwl	$9,7($2)
        if re.match(rf"^.*\(\s*{r_source}\s*\)$", rest):
            return True

    elif op in store_mnemonics:
        if re.match(rf"^.*\(\s*{r_source}\s*\)$", rest):
            return True
        # "line_loads_from_reg" is a bit of a lie
        if re.match(rf"^{r_source},.*$", rest):
            return True

    elif op == "lwc2":
        # lwc2 $5, 4( $4
        if re.match(rf"^.*\(\s*{r_source}\s*\)$", rest):
            return True

    elif op == "jal":
        if re.match(rf"^.*,\s*{r_source}$", rest):
            return True

    elif op == "j":
        if re.match(rf"^{r_source}$", rest):
            return True

    elif op in ("ctc2", "mtc0", "mtc2"):
        if re.match(rf"^{r_source},.*$", rest):
            return True

    elif op in ("mtlo", "mthi"):
        if re.match(rf"^{r_source}$", rest):
            return True

    elif op in branch_mnemonics:
        if re.match(rf"^{r_source},.*$", rest):
            return True
        if re.match(rf"^.*,\s*{r_source},.*$", rest):
            return True

    elif op in single_reg_loads:
        if re.match(rf"^.*,\s*{r_source}$", rest):
            return True
        if op.startswith("mult"):
            if re.match(rf"^{r_source},.*$", rest):
                return True
        if op.startswith("div") or op.startswith("rem"):
            # e.g. div	$3,$3,$7
            if re.match(rf"^.*,{r_source}.*$", rest):
                return True

    elif op in double_reg_loads:
        if re.match(rf"^.*,\s*{r_source},.*$", rest):
            return True
        if re.match(rf"^.*,.*,\s*{r_source}$", rest):
            return True

    return False


def is_number(value: str) -> bool:
    if re.match(r"^-?\d+$", value) or re.match(r"^-?0x[A-Fa-f0-9]+$", value):
        return True
    return False


def uses_at(line: str) -> bool:
    line = strip_comments(line)

    # sw	$2,%lo(s_attr)($3)
    if match := re.match(r"^s[wbh]\s+(\$[a-z0-9]+),\s*%lo\(([^(]+)\)\(([^)]+)\)", line):
        return False

    # sw	$2,D_801813A4
    # sw	$3,g_CurrentRoom+40
    # sw	$2,D_us_8017863C.4
    if match := re.match(r"^s[wbh]\s+(\$[a-z0-9]+),\s*(-?[A-z0-9_.+]+)$", line):
        operand = match.group(2)
        if not is_number(operand):
            return True

    # sb	$2,g_InputSaveName($3)
    # sw	$2,-26($16)
    elif match := re.match(r"^s[wbh]\s+(\$[a-z0-9]+),\s*([^(]+)\(([^)]+)\)", line):
        operand = match.group(2)

    # lw	$2,-1000($16)
    elif match := re.match(r"l[a-z]+\s+(\$[a-z0-9]+),\s*([^(]+)\(([^)]+)\)", line):
        operand = match.group(2)

    else:
        return False

    if is_number(operand):
        num = int(operand, 0)
        if -32769 < num < 32768:
            return False

    return True


def parse_load_or_store(rest: str):
    if match := re.match(r"(\$[a-z0-9]+),\s*%lo\(([^(]+)\)\(([^(]+)\)", rest):
        r_dest, operand, r_source = match.group(1, 2, 3)
        needs_expanding = False
    elif match := re.match(r"(\$[a-z0-9]+),\s*([^(]+)\(([^)]+)\)", rest):
        r_dest, operand, r_source = match.group(1, 2, 3)
        needs_expanding = True
    elif match := re.match(r"(\$[a-z0-9]+),\s*([^(]+)", rest):
        r_dest, operand = match.group(1, 2)
        r_source = None
        needs_expanding = True
    else:
        raise Exception(f"Unable to parse load/store instruction: {rest}")

    if re.match(r"^-?\d+$", operand) or re.match(r"^-?0x[A-Fa-f0-9]+$", operand):
        is_addend = False
    else:
        is_addend = True

    return (r_source, r_dest, operand, is_addend, needs_expanding)


def div_needs_expanding(line: str) -> bool:
    inst, *rest = line.split()
    if not (inst.startswith("div") or inst.startswith("rem")):
        return False

    r_dest, *_ = rest[0].split(",")
    return r_dest not in ("$zero", "$0")


def expand_load_immediate(line: str) -> List[str]:
    res = []

    match = re.match(r"li\s+(\$[0-9A-z]+),\s?(-?[x0-9a-fA-F]+)", line)
    assert match is not None, "li regex failed"

    r_dest = match.group(1)
    operand = int(match.group(2), 0)

    if 0 < operand < 0x10000:
        res.append(f"ori\t{r_dest},$zero,{operand}")
    elif operand >= 0x10000:
        res.append(f"lui\t{r_dest},({operand} >> 16) & 0xFFFF")
        if operand & 0xFFFF:
            res.append(f"ori\t{r_dest},{r_dest},{operand} & 0xFFFF")
    elif 0 > operand > -0x8000:
        res.append(f"addiu\t{r_dest},$zero,{operand}")
    elif operand == -0x8000:
        res.append(f"addiu\t{r_dest},$zero,{operand} & 0xFFFF")
    elif operand < -0x8000:
        res.append(f"lui\t{r_dest},({operand} >> 16) & 0xFFFF")
        if operand & 0xFFFF:
            res.append(f"ori\t{r_dest},{r_dest},{operand} & 0xFFFF")
    else:
        # TODO: raise an exception here instead?
        # ori is actually addiu on ASPSX 2.56+
        res.append(f"ori\t{r_dest},0")

    return res


def expand_move(line: str):
    line = strip_comments(line)
    op, *rest = line.split()
    if op == "move":
        args = " ".join(rest)
        r_dest, r_source = args.split(",")
        return f"addu\t{r_dest},{r_source},$zero"
    return line


def is_label(line: str):
    return re.match(r"\$L(b|e)?\d+:$", line)


def is_instruction(line: str, ignore_nop=False, ignore_set=False, ignore_label=False):
    if len(line) == 0:
        return False

    if ignore_nop and line == "#nop":
        return False
    if ignore_set and line in (
        ".set\treorder",
        ".set\tnoreorder",
        ".set\tvolatile",
        ".set\tnovolatile",
    ):
        return False
    if ignore_label and is_label(line):
        return False

    if line.startswith(".stab"):
        return False
    if line.startswith(".def") or line.startswith(".bend") or line.startswith(".begin"):
        return False
    if line.startswith(".loc"):
        return False

    if line.startswith("L") and line[1] not in "0123456789" and line.endswith(":"):
        return False
    if line in (".set\tmacro", ".set\tnomacro"):
        return False
    if line in ("#.set\tvolatile", "#.set\tnovolatile"):
        return False
    if line in ("#APP", "#NO_APP"):
        return False

    return True


def get_next_register(reg: str):
    lut = {
        # li $fx
        "$f0": "$f1",
        "$f2": "$f3",
        "$f4": "$f5",
        "$f6": "$f7",
        "$f12": "$f13",
        "$f14": "$f15",
        # names
        "$v0": "$v1",
        "$a0": "$a1",
        "$a2": "$a3",
        "$t0": "$t1",
        "$t2": "$t3",
        "$s0": "$s1",
        "$s2": "$s3",
        # nums
        "$2": "$3",  # $v0
        "$4": "$5",  # $a0
        "$6": "$7",  # $a2
        "$8": "$9",  # $t0
        "$10": "$11",  # t2
        "$18": "$19",  # s2
    }
    next_reg = lut.get(reg)
    if next_reg is not None:
        return next_reg
    # Generic fallback for numeric registers ($N): a 64-bit double occupies the
    # pair ($N, $N+1), so the high word always lands in $N+1. The named-register
    # and $f cases are handled by the LUT above; this covers even GP regs the LUT
    # never enumerated (e.g. $12/$14/$16 -> cc1 emits these as `li.d $16,...`).
    m = re.fullmatch(r"\$(\d+)", reg)
    if m is not None:
        return f"${int(m.group(1)) + 1}"
    raise AssertionError(f"Unknown mapping for {reg}")


def expand_macro(line: str):
    res = []
    for l in strip_comments(line).split(";"):
        l = l.strip()
        if len(l) == 0:
            continue
        op, *rest = l.split()
        res.append(
            (op, " ".join(rest)),
        )
    return res


def load_immediate_single(line: str):
    res = []
    r1, value = line[5:].split(",")
    (num,) = struct.unpack(">i", struct.pack(">f", float(value)))
    upper = (num & 0xFFFF_0000) >> 16
    lower = (num & 0x0000_FFFF) >> 0

    res.append(f"lui\t{r1},0x{upper:X}")
    # we don't always need the lower part
    if lower:
        res.append(f"ori\t{r1},0x{lower:X}")
    return res


def load_immediate_double(line: str):
    res = []
    r1, value = line[5:].split(",")
    r2 = get_next_register(r1)
    (num,) = struct.unpack(">q", struct.pack(">d", float(value)))

    r1_upper = (num & 0x0000_0000_FFFF_0000) >> 16
    r1_lower = (num & 0x0000_0000_0000_FFFF) >> 0
    r2_upper = (num & 0xFFFF_0000_0000_0000) >> 48
    r2_lower = (num & 0x0000_FFFF_0000_0000) >> 32

    if r1_upper or r1_lower:
        res.append(f"lui\t{r1},0x{r1_upper:X}")
        if r1_lower:
            res.append(f"ori\t{r1},0x{r1_lower:X}")
    else:
        res.append(f"li\t{r1},0x0")

    res.append(f"lui\t{r2},0x{r2_upper:X}")
    if r2_lower:
        res.append(f"ori\t{r2},0x{r2_lower:X}")

    return res
