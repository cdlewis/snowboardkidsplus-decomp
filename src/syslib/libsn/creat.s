# Handwritten assembly reference:
# https://github.com/sozud/psy-q/blob/2266d676f8de8a7e7a96d9cbb92854f0a55a86a1/3.6/PSYQ/SRC/LIBSN/CREAT.S
.include "macro.inc"

.set noat
.set noreorder

.section .text, "ax"

glabel PCcreat
    addu       $a1, $a0, $zero
    addu       $a2, $zero, $zero
    break      0, 258
    beqz       $v0, .L800B5400
     addu      $v0, $v1, $zero
    addiu      $v0, $zero, -0x1
.L800B5400:
    jr         $ra
     nop
endlabel PCcreat
