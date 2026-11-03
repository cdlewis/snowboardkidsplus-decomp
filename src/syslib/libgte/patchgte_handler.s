# Handwritten exception-patch payload reference:
# https://github.com/ser-pounce/rood-reverse/blob/8bd7908d878824d6cfa02ce65e887dfaf586f11f/src/SLUS_010.40/libgte/PATCHGTE.s
.include "macro.inc"

.set noat
.set noreorder

.section .text, "ax"

glabel func_800BF0D4
    sw $at, 0x4($k0)
    sw $v0, 0x8($k0)
    sw $v1, 0xC($k0)
    sw $ra, 0x7C($k0)
    mfc0 $v1, $14
    nop
alabel gteExceptionPatchReplacement
    sw $at, 0x4($k0)
    sw $v0, 0x8($k0)
    mfc0 $v0, $13
    sw $v1, 0xC($k0)
    mfc0 $v1, $14
    sw $ra, 0x7C($k0)
endlabel func_800BF0D4
alabel gteExceptionPatchEnd
    nop
