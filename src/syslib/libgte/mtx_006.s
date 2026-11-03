# Handwritten assembly reference:
# https://github.com/ReGame-Labs/dcb_decomp/blob/6c9caf124450cafa1b5f402d03d82b90bb02d16e/src/main/psyq/libgte_msc01.s
.include "macro.inc"
.include "gte_macros.inc"

.set noat
.set noreorder

.section .text, "ax"

glabel PushMatrix
    lui        $t6, %hi(matrixStackOffset)
    lw         $t6, %lo(matrixStackOffset)($t6)
    nop
    slti       $at, $t6, 0x280
    bnez       $at, .L800BE1C8
     nop
    lui        $at, %hi(matrixStackSavedRa)
    sw         $ra, %lo(matrixStackSavedRa)($at)
    lui        $a0, %hi(matrixStackOverflowMessage)
    jal        printf
     addiu     $a0, $a0, %lo(matrixStackOverflowMessage)
    lui        $ra, %hi(matrixStackSavedRa)
    lw         $ra, %lo(matrixStackSavedRa)($ra)
    nop
    jr         $ra
     nop
.L800BE1C8:
    lui        $t7, %hi(matrixStack)
    addiu      $t7, $t7, %lo(matrixStack)
    addu       $t7, $t7, $t6
    cfc2       $t0, $0
    cfc2       $t1, $1
    sw         $t0, 0x0($t7)
    sw         $t1, 0x4($t7)
    cfc2       $t0, $2
    cfc2       $t1, $3
    sw         $t0, 0x8($t7)
    sw         $t1, 0xC($t7)
    cfc2       $t0, $4
    nop
    sw         $t0, 0x10($t7)
    cfc2       $t0, $5
    cfc2       $t1, $6
    cfc2       $t2, $7
    sw         $t0, 0x14($t7)
    sw         $t1, 0x18($t7)
    sw         $t2, 0x1C($t7)
    addi       $t6, $t6, 0x20
    lui        $at, %hi(matrixStackOffset)
    sw         $t6, %lo(matrixStackOffset)($at)
    jr         $ra
     nop
endlabel PushMatrix

glabel PopMatrix
    lui        $t6, %hi(matrixStackOffset)
    lw         $t6, %lo(matrixStackOffset)($t6)
    nop
    bgtz       $t6, .L800BE268
     nop
    lui        $at, %hi(matrixStackSavedRa)
    sw         $ra, %lo(matrixStackSavedRa)($at)
    lui        $a0, %hi(matrixStackUnderflowMessage)
    jal        printf
     addiu     $a0, $a0, %lo(matrixStackUnderflowMessage)
    lui        $ra, %hi(matrixStackSavedRa)
    lw         $ra, %lo(matrixStackSavedRa)($ra)
    nop
    jr         $ra
     nop
.L800BE268:
    addi       $t6, $t6, -0x20
    lui        $at, %hi(matrixStackOffset)
    sw         $t6, %lo(matrixStackOffset)($at)
    lui        $t7, %hi(matrixStack)
    addiu      $t7, $t7, %lo(matrixStack)
    addu       $t7, $t7, $t6
    lw         $t0, 0x0($t7)
    lw         $t1, 0x4($t7)
    ctc2       $t0, $0
    ctc2       $t1, $1
    lw         $t0, 0x8($t7)
    lw         $t1, 0xC($t7)
    ctc2       $t0, $2
    ctc2       $t1, $3
    lw         $t0, 0x10($t7)
    nop
    ctc2       $t0, $4
    nop
    lw         $t0, 0x14($t7)
    lw         $t1, 0x18($t7)
    lw         $t2, 0x1C($t7)
    ctc2       $t0, $5
    ctc2       $t1, $6
    ctc2       $t2, $7
    jr         $ra
     nop
endlabel PopMatrix
    nop
    nop
