# Copy a GPU packet to GsOUT_PACKET_P and prepend it to an ordering-table entry.
# The source tag holds the payload word count in its high byte with a zero link.
# The trapping addi/sub instructions suggest handwritten assembly. The target
# compiler emitted non-trapping arithmetic in the C attempts. The loop also
# shares the jump's delay-slot store with the exit path.
# No known callers. The original executable has no direct branches, jumps, or
# aligned pointers from outside this routine into it.

.include "macro.inc"

.set noreorder
.set noat
.text

glabel func_80058560
    lui     $t9, %hi(GsOUT_PACKET_P)
    addiu   $t9, $t9, %lo(GsOUT_PACKET_P)
    lw      $t0, 0($a1)
    lw      $v1, 0($t9)
    lw      $t4, 0($a0)
    lui     $t8, 0x00FF
    ori     $t8, $t8, 0xFFFF
    and     $v0, $t8, $v1
    and     $t4, $t8, $t4
    or      $t4, $t0, $t4
    sw      $t4, 0($v1)
    sw      $v0, 0($a0)
    lui     $v0, 0x0100
.LcopyPayload:
    addi    $v1, $v1, 4
    addi    $a1, $a1, 4
    # The delay slot reads one extra source word when the count reaches zero.
    beqz    $t0, .LupdateCursor
     lw     $t4, 0($a1)
    sub     $t0, $t0, $v0
    sw      $t4, 0($v1)
    j       .LcopyPayload
.LupdateCursor:
     sw     $v1, 0($t9)
    jr      $ra
     nop
endlabel func_80058560
