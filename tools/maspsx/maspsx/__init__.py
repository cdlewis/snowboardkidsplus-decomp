"""PSY-Q assembler compatibility, with stable package-level imports."""

from .instructions import (
    strip_comments,
    line_loads_from_reg,
    is_number,
    uses_at,
    parse_load_or_store,
    div_needs_expanding,
    expand_load_immediate,
    expand_move,
    is_label,
    is_instruction,
    get_next_register,
    expand_macro,
    load_immediate_single,
    load_immediate_double,
    branch_mnemonics,
    jump_mnemonics,
    load_mnemonics,
    store_mnemonics,
    single_reg_loads,
    double_reg_loads,
)
from .processor import MaspsxProcessor
