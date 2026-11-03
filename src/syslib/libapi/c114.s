# Handwritten assembly reference:
# https://github.com/Xeeynamo/sotn-decomp/blob/0bb77822d1cbd6d22f39253ea6ba8303efdf8d28/src/main/psxsdk/libapi/c114.c
.include "syslib/bios.inc"

.section .text, "ax"

dlabel func_800D17D8
     .word 0x21007350
     .word 0x004236AD
enddlabel func_800D17D8

BIOS_CALL _96_remove, 0xA0, 0x72
    nop
    nop
