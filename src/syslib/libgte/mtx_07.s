# Handwritten assembly reference:
# https://github.com/ser-pounce/rood-reverse/blob/8bd7908d878824d6cfa02ce65e887dfaf586f11f/src/SLUS_010.40/libgte/MTX_07.s
.include "macro.inc"
.include "gte_macros.inc"

.set noat
.set noreorder

.section .text, "ax"

glabel TransMatrix
    lw         $t0, 0x0($a1)
    lw         $t1, 0x4($a1)
    lw         $t2, 0x8($a1)
    sw         $t0, 0x14($a0)
    sw         $t1, 0x18($a0)
    sw         $t2, 0x1C($a0)
    addu       $v0, $a0, $zero
    jr         $ra
     nop
endlabel TransMatrix
    nop
    nop
    nop
