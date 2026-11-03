# Handwritten assembly reference:
# https://github.com/sozud/psy-q/blob/2266d676f8de8a7e7a96d9cbb92854f0a55a86a1/3.6/PSYQ/SRC/LIBSN/SNREAD.S
.include "macro.inc"

.set noat
.set noreorder

.section .text, "ax"

glabel _SN_read
    break      0, 261
    beqz       $v0, .L800B54D8
     addu      $v0, $v1, $zero
    addiu      $v0, $zero, -0x1
.L800B54D8:
    jr         $ra
     nop
endlabel _SN_read
