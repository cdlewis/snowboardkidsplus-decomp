# Handwritten assembly reference:
# https://github.com/Xeeynamo/sotn-decomp/blob/0bb77822d1cbd6d22f39253ea6ba8303efdf8d28/src/main/psxsdk/libcard/c172.c
.include "syslib/bios.inc"

.section .text, "ax"

BIOS_CALL _card_load, 0xA0, 0xAC
