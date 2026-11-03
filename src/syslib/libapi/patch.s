# Handwritten assembly reference:
# https://github.com/ReGame-Labs/dcb_decomp/blob/6c9caf124450cafa1b5f402d03d82b90bb02d16e/src/main/psyq/libapi_patch.s
.include "macro.inc"

.set noat
.set noreorder

.section .text, "ax"

glabel EnablePAD
    lui        $t1, %hi(jtbl_80141670)
    lw         $t1, %lo(jtbl_80141670)($t1)
    nop
    jr         $t1
     nop
endlabel EnablePAD

glabel DisablePAD
    lui        $t1, %hi(jtbl_80141674)
    lw         $t1, %lo(jtbl_80141674)($t1)
    nop
    jr         $t1
     nop
endlabel DisablePAD

glabel _patch_pad
    lui        $at, %hi(padPatchSavedRa)
    sw         $ra, %lo(padPatchSavedRa)($at)
    jal        EnterCriticalSection
     nop
    addiu      $t1, $zero, 0x57
    addiu      $t2, $zero, 0xB0
    jalr       $t2
     nop
    lw         $v0, 0x16C($v0)
    addiu      $t1, $zero, 0xB
    addi       $v1, $v0, 0x884
    lui        $at, %hi(jtbl_80141670)
    sw         $v1, %lo(jtbl_80141670)($at)
    addi       $v1, $v0, 0x894
    lui        $at, %hi(jtbl_80141674)
    sw         $v1, %lo(jtbl_80141674)($at)
.L800D6FF0:
    sw         $zero, 0x594($v0)
    addiu      $v0, $v0, 0x4
    addiu      $t1, $t1, -0x1
    bnez       $t1, .L800D6FF0
     nop
    jal        FlushCache
     nop
    lui        $ra, %hi(padPatchSavedRa)
    lw         $ra, %lo(padPatchSavedRa)($ra)
    nop
    jr         $ra
     nop
endlabel _patch_pad
    nop
    nop
