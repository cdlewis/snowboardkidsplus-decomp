# Handwritten assembly reference:
# https://github.com/sozud/psy-q/blob/2266d676f8de8a7e7a96d9cbb92854f0a55a86a1/3.6/PSYQ/SRC/LIBSN/LSEEK.S
.include "macro.inc"

.set noat
.set noreorder

.section .text, "ax"

glabel PClseek
    addu       $a3, $a2, $zero
    addu       $a2, $a1, $zero
    addu       $a1, $a0, $zero
    break      0, 263
    beqz       $v0, .L800B53E0
     addu      $v0, $v1, $zero
    addiu      $v0, $zero, -0x1
.L800B53E0:
    jr         $ra
     nop
endlabel PClseek
