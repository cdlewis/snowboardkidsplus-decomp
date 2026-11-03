# Handwritten assembly reference:
# https://github.com/Xeeynamo/sotn-decomp/blob/0bb77822d1cbd6d22f39253ea6ba8303efdf8d28/src/main/psxsdk/libc/a63.c
.include "syslib/bios.inc"

.section .text, "ax"

dlabel func_800D0DC8
     .word 0x21017350
     .word 0x004237AD
enddlabel func_800D0DC8

BIOS_CALL puts, 0xB0, 0x3F
    nop
    nop
