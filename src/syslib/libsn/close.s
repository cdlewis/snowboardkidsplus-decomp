# Handwritten assembly reference:
# https://github.com/sozud/psy-q/blob/2266d676f8de8a7e7a96d9cbb92854f0a55a86a1/3.6/PSYQ/SRC/LIBSN/CLOSE.S
.include "macro.inc"

.set noat
.set noreorder

.section .text, "ax"

glabel PCclose
    addu       $a1, $a0, $zero
    break      0, 260
    jr         $ra
     nop
endlabel PCclose
