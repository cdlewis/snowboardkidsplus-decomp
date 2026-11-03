# Handwritten assembly reference:
# https://github.com/sozud/psy-q/blob/2266d676f8de8a7e7a96d9cbb92854f0a55a86a1/3.6/PSYQ/SRC/LIBSN/OPEN.S
.include "macro.inc"

.set noat
.set noreorder

.section .text, "ax"

glabel PCopen
    addu       $a2, $a1, $zero
    addu       $a1, $a0, $zero
    break      0, 259
    beqz       $v0, .L800B53AC
     addu      $v0, $v1, $zero
    addiu      $v0, $zero, -0x1
.L800B53AC:
    jr         $ra
     nop
endlabel PCopen
