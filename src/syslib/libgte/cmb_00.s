# Handwritten assembly reference:
# https://github.com/ser-pounce/rood-reverse/blob/8bd7908d878824d6cfa02ce65e887dfaf586f11f/src/SLUS_010.40/libgte/CMB_00.s
.include "macro.inc"
.include "gte_macros.inc"

.set noat
.set noreorder

.section .text, "ax"

glabel RotTransPers4
    lwc2       $0, 0x0($a0)
    lwc2       $1, 0x4($a0)
    lwc2       $2, 0x0($a1)
    lwc2       $3, 0x4($a1)
    lwc2       $4, 0x0($a2)
    lwc2       $5, 0x4($a2)
    nop
    rtpt
    lw         $t0, 0x10($sp)
    lw         $t1, 0x14($sp)
    lw         $t2, 0x18($sp)
    swc2       $12, 0x0($t0)
    swc2       $13, 0x0($t1)
    swc2       $14, 0x0($t2)
    cfc2       $v1, $31
    lwc2       $0, 0x0($a3)
    lwc2       $1, 0x4($a3)
    nop
    rtps
    lw         $t0, 0x1C($sp)
    lw         $t1, 0x20($sp)
    lw         $t2, 0x24($sp)
    swc2       $14, 0x0($t0)
    swc2       $8, 0x0($t1)
    cfc2       $t0, $31
    mfc2       $v0, $19
    or         $t0, $t0, $v1
    sw         $t0, 0x0($t2)
    jr         $ra
     sra       $v0, $v0, 2
endlabel RotTransPers4
    nop
    nop
