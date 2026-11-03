# Handwritten assembly reference:
# https://github.com/ser-pounce/rood-reverse/blob/8bd7908d878824d6cfa02ce65e887dfaf586f11f/src/SLUS_010.40/libgte/SMP_02.s
.include "macro.inc"
.include "gte_macros.inc"

.set noat
.set noreorder

.section .text, "ax"

glabel RotTransPers
    lwc2       $0, 0x0($a0)
    lwc2       $1, 0x4($a0)
    nop
    rtps
    swc2       $14, 0x0($a1)
    swc2       $8, 0x0($a2)
    cfc2       $v1, $31
    mfc2       $v0, $19
    sw         $v1, 0x0($a3)
    jr         $ra
     sra       $v0, $v0, 2
endlabel RotTransPers
    nop
