# Handwritten assembly reference:
# https://github.com/Xeeynamo/sotn-decomp/blob/0bb77822d1cbd6d22f39253ea6ba8303efdf8d28/src/main/psxsdk/libapi/a21.c
.include "syslib/bios.inc"

.section .text, "ax"

BIOS_CALL PAD_init2, 0xB0, 0x15
