# Scratchpad TMD renderer. No callers of the entry point are known.
# The wrapper saves the caller's registers in inline storage. Its two
# helpers share live registers and overwrite saved registers without their
# own stack frames, which identifies the private handwritten convention.

.include "macro.inc"

.set noreorder
.set noat
.text

# $a0 is the ordering table and $a1 is the TMD object.
# The wrapper sets up $t6/$t7 as scratchpad pointers and $s0-$s3 as model state.
glabel func_800585BC
    lui     $t9, %hi(.LsavedRegisters)
    addiu   $t9, $t9, %lo(.LsavedRegisters)
    sw      $s0, 0x0($t9)
    sw      $s1, 0x4($t9)
    sw      $s2, 0x8($t9)
    sw      $s3, 0xC($t9)
    sw      $s4, 0x10($t9)
    sw      $s5, 0x14($t9)
    sw      $s6, 0x18($t9)
    sw      $s7, 0x1C($t9)
    sw      $ra, 0x20($t9)
    lui     $t6, (0x1F800000 >> 16)
    lui     $t7, (0x1F800020 >> 16)
    ori     $t7, $t7, (0x1F800020 & 0xFFFF)
    lw      $s0, 0x0($a1)
    lw      $s1, 0x8($a1)
    lw      $s2, 0x10($a1)
    lw      $s3, 0x14($a1)
    jal     func_80058664
     nop
    lui     $t9, %hi(.LsavedRegisters)
    addiu   $t9, $t9, %lo(.LsavedRegisters)
    lw      $s0, 0x0($t9)
    lw      $s1, 0x4($t9)
    lw      $s2, 0x8($t9)
    lw      $s3, 0xC($t9)
    lw      $s4, 0x10($t9)
    lw      $s5, 0x14($t9)
    lw      $s6, 0x18($t9)
    lw      $ra, 0x20($t9)
    lw      $s7, 0x1C($t9)
    jr      $ra
     nop
endlabel func_800585BC

.LsavedRegisters:
    .space 0x24

# Unpack and project TMD primitives. Pass GTE depth to the packet writer in $v1.
glabel func_80058664
    lui     $t9, %hi(.LsavedReturnAddress)
    addiu   $t9, $t9, %lo(.LsavedReturnAddress)
    sw      $ra, 0x0($t9)
.LnextPrimitive:
    lw      $t9, 0x0($s2)
    addiu   $t8, $t6, 0x0
    lui     $at, (0xFE000000 >> 16)
    and     $t0, $t9, $at
    sw      $t0, 0x0($t8)
    srl     $t0, $t9, 24
    srl     $t1, $t9, 16
    andi    $t1, $t1, 0xFF
    addi    $s2, $s2, 0x4
    andi    $t9, $t0, 0x4
    beqz    $t9, .LcolorCount
     andi   $v0, $t0, 0x8
    srl     $v0, $v0, 3
    addi    $v0, $v0, 0x3
    addiu   $t8, $t7, 0x20
.LcopyUvs:
    lw      $t9, 0x0($s2)
    addi    $s2, $s2, 0x4
    sw      $t9, 0x0($t8)
    addi    $v0, $v0, -0x1
    addi    $t8, $t8, 0x4
    bnez    $v0, .LcopyUvs
     nop
.LcolorCount:
    andi    $t9, $t0, 0x10
    beqz    $t9, .LbeginColors
     ori    $v0, $zero, 0x1
    andi    $v0, $t0, 0x8
    srl     $v0, $v0, 3
    addi    $v0, $v0, 0x3
.LbeginColors:
    addiu   $t8, $t7, 0x0
.LcopyColors:
    lw      $t9, 0x0($s2)
    addi    $s2, $s2, 0x4
    sw      $t9, 0x0($t8)
    addi    $v0, $v0, -0x1
    addi    $t8, $t8, 0x4
    bnez    $v0, .LcopyColors
     nop
    lh      $t8, 0x0($s2)
    nop
    sll     $t8, $t8, 3
    add     $t9, $s0, $t8
    lwc2    $0, 0x0($t9)
    lwc2    $1, 0x4($t9)
    addi    $s2, $s2, 0x2
    lh      $t8, 0x0($s2)
    nop
    sll     $t8, $t8, 3
    add     $t9, $s0, $t8
    lwc2    $2, 0x0($t9)
    lwc2    $3, 0x4($t9)
    addi    $s2, $s2, 0x2
    lh      $t8, 0x0($s2)
    nop
    sll     $t8, $t8, 3
    add     $t9, $s0, $t8
    lwc2    $4, 0x0($t9)
    lwc2    $5, 0x4($t9)
    nop
    nop
    rtpt
    swc2    $12, 0x10($t7)
    swc2    $13, 0x14($t7)
    swc2    $14, 0x18($t7)
    nop
    nop
    nclip
    swc2    $24, 0x4($t6)
    nop
    andi    $t9, $t0, 0x8
    beqz    $t9, .LprojectFourthVertex
     nop
    nop
    avsz3
.LprojectFourthVertex:
    beqz    $t9, .LadvanceIndices
     nop
    addi    $s2, $s2, 0x2
    lh      $t8, 0x0($s2)
    nop
    sll     $t8, $t8, 3
    add     $t9, $s0, $t8
    lwc2    $0, 0x0($t9)
    lwc2    $1, 0x4($t9)
    nop
    nop
    rtps
    swc2    $14, 0x1C($t7)
    nop
    nop
    avsz4
.LadvanceIndices:
    addi    $s2, $s2, 0x2
    andi    $t9, $s2, 0x3
    beqz    $t9, .LtestWinding
     nop
    addi    $s2, $s2, 0x2
.LtestWinding:
    lw      $t9, 0x4($t6)
    nop
    bltz    $t9, .LfinishPrimitive
     mfc2   $v1, $7
    nop
    mtc2    $v0, $31
    nop
    lui     $at, (0xFE018000 >> 16)
    ori     $at, $at, (0xFE018000 & 0xFFFF)
    and     $v0, $v0, $at
    bnez    $v0, .LfinishPrimitive
     nop
    jal     func_80058844
     nop
.LfinishPrimitive:
    addi    $s3, $s3, -0x1
    bnez    $s3, .LnextPrimitive
     nop
    lui     $t9, %hi(.LsavedReturnAddress)
    addiu   $t9, $t9, %lo(.LsavedReturnAddress)
    lw      $ra, 0x0($t9)
    nop
    jr      $ra
     nop
endlabel func_80058664

.LsavedReturnAddress:
    .word   0

# Emit a GPU packet from scratchpad colors, projected XY, and UV words.
# Read the ordering table from $a0 and depth from $v1. The wrapper preserves
# $s5-$s7 for the caller, so the writer can use them for the count and cursors.
glabel func_80058844
    lw      $t1, 0x0($t6)
    nop
    lui     $at, (0x8000000 >> 16)
    and     $t2, $t1, $at
    lui     $at, (0x10000000 >> 16)
    and     $t3, $t1, $at
    lui     $at, (0x4000000 >> 16)
    and     $t4, $t1, $at
    lui     $t9, %hi(GsOUT_PACKET_P)
    addiu   $t9, $t9, %lo(GsOUT_PACKET_P)
    lw      $s7, 0x0($t9)
    nop
    addu    $s6, $s7, $zero
    addi    $s7, $s7, 0x4
    lw      $t9, 0x0($t7)
    addi    $s5, $zero, 0x1
    lui     $at, (0xFFFFFF >> 16)
    ori     $at, $at, (0xFFFFFF & 0xFFFF)
    and     $t9, $t9, $at
    or      $t9, $t9, $t1
    sw      $t9, 0x0($s7)
    addi    $s7, $s7, 0x4
    lw      $t9, 0x10($t7)
    addi    $s5, $s5, 0x1
    sw      $t9, 0x0($s7)
    addi    $s7, $s7, 0x4
    beqz    $t4, .Lvertex1Color
     lw     $t9, 0x20($t7)
    addi    $s5, $s5, 0x1
    sw      $t9, 0x0($s7)
    addi    $s7, $s7, 0x4
.Lvertex1Color:
    beqz    $t3, .Lvertex1Position
     lw     $t9, 0x4($t7)
    addi    $s5, $s5, 0x1
    sw      $t9, 0x0($s7)
    addi    $s7, $s7, 0x4
.Lvertex1Position:
    lw      $t9, 0x14($t7)
    addi    $s5, $s5, 0x1
    sw      $t9, 0x0($s7)
    addi    $s7, $s7, 0x4
    beqz    $t4, .Lvertex2Color
     lw     $t9, 0x24($t7)
    addi    $s5, $s5, 0x1
    sw      $t9, 0x0($s7)
    addi    $s7, $s7, 0x4
.Lvertex2Color:
    beqz    $t3, .Lvertex2Position
     lw     $t9, 0x8($t7)
    addi    $s5, $s5, 0x1
    sw      $t9, 0x0($s7)
    addi    $s7, $s7, 0x4
.Lvertex2Position:
    lw      $t9, 0x18($t7)
    addi    $s5, $s5, 0x1
    sw      $t9, 0x0($s7)
    addi    $s7, $s7, 0x4
    beqz    $t4, .LquadVertex
     lw     $t9, 0x28($t7)
    addi    $s5, $s5, 0x1
    sw      $t9, 0x0($s7)
    addi    $s7, $s7, 0x4
.LquadVertex:
    beqz    $t2, .LlinkPacket
     nop
    beqz    $t3, .Lvertex3Position
     lw     $t9, 0xC($t7)
    addi    $s5, $s5, 0x1
    sw      $t9, 0x0($s7)
    addi    $s7, $s7, 0x4
.Lvertex3Position:
    lw      $t9, 0x1C($t7)
    addi    $s5, $s5, 0x1
    sw      $t9, 0x0($s7)
    addi    $s7, $s7, 0x4
    beqz    $t4, .LlinkPacket
     lw     $t9, 0x2C($t7)
    addi    $s5, $s5, 0x1
    sw      $t9, 0x0($s7)
    addi    $s7, $s7, 0x4
.LlinkPacket:
    lui     $t9, %hi(GsOUT_PACKET_P)
    addiu   $t9, $t9, %lo(GsOUT_PACKET_P)
    sw      $s7, 0x0($t9)
    sll     $s5, $s5, 24
    sra     $v1, $v1, 2
    sll     $v1, $v1, 2
    add     $v1, $v1, $a0
    lw      $t9, 0x0($v1)
    lui     $t8, (0xFFFFFF >> 16)
    ori     $t8, $t8, (0xFFFFFF & 0xFFFF)
    and     $t9, $t9, $t8
    or      $t9, $t9, $s5
    sw      $t9, 0x0($s6)
    and     $t9, $s6, $t8
    sw      $t9, 0x0($v1)
    jr      $ra
     nop
endlabel func_80058844

# Preserve the padding before the next unit.
.space 0x3C
