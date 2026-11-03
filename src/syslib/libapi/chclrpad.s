# Handwritten assembly reference:
# https://github.com/ser-pounce/rood-reverse/blob/8bd7908d878824d6cfa02ce65e887dfaf586f11f/src/SLUS_010.40/libapi/CHCLRPAD.s
.include "macro.inc"

.set noat
.set noreorder

.section .text, "ax"

glabel _remove_ChgclrPAD
    lui        $at, %hi(changeClearPadSavedRa)
    sw         $ra, %lo(changeClearPadSavedRa)($at)
    jal        EnterCriticalSection
     nop
    addiu      $t1, $zero, 0x57
    addiu      $t2, $zero, 0xB0
    jalr       $t2
     nop
    addiu      $t2, $zero, 0x9
    lw         $v0, 0x16C($v0)
    nop
    addi       $v1, $v0, 0x62C
.L800D7058:
    sw         $zero, 0x0($v1)
    addiu      $v1, $v1, 0x4
    addiu      $t2, $t2, -0x1
    bnez       $t2, .L800D7058
     nop
    jal        FlushCache
     nop
    jal        ExitCriticalSection
     nop
    lui        $ra, %hi(changeClearPadSavedRa)
    lw         $ra, %lo(changeClearPadSavedRa)($ra)
    nop
    jr         $ra
     nop
endlabel _remove_ChgclrPAD
    nop
    nop
