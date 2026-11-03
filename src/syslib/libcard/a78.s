# Handwritten assembly reference:
# https://github.com/Xeeynamo/sotn-decomp/blob/0bb77822d1cbd6d22f39253ea6ba8303efdf8d28/src/main/psxsdk/libcard/a78.c
.include "syslib/bios.inc"

.section .text, "ax"

BIOS_CALL _card_write, 0xB0, 0x4E
