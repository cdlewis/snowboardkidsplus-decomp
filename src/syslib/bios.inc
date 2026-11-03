# PSX BIOS entry stubs, following Tomba's DEFINE_FN / DEFINE_SYS_FN pattern:
# https://github.com/hansbonini/psx_tomba/blob/b0aab7ef43785b257f66fb007df36b222e1a3665/include/psyq/bios.inc
.include "macro.inc"

.set noat
.set noreorder

.macro BIOS_CALL name, vector, number
    glabel \name
    addiu $t2, $zero, \vector
    jr $t2
     addiu $t1, $zero, \number
    endlabel \name
    nop
.endm

.macro BIOS_SYSCALL name, number
    glabel \name
    addiu $a0, $zero, \number
    syscall 0
    jr $ra
     nop
    endlabel \name
.endm
