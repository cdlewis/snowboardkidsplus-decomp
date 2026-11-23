# Apparently unused handwritten stub that switches to the scratchpad stack.
# It overwrites the saved return address and leaves the original stack pointer
# in .LsavedSp. The final jump uses the next function's prologue as its delay slot.

.include "macro.inc"

.set noreorder
.set noat
.text

glabel func_800589F0
    lui     $t9, %hi(.LsavedSp)
    addiu   $t9, $t9, %lo(.LsavedSp)
    sw      $sp, 0($t9)
    lui     $sp, 0x1F80
    ori     $sp, $sp, 0x03FC
    lui     $s0, 0x1F80
.LsavedSp:
    .word   0
    addiu   $sp, $sp, -0x2C
    sw      $ra, 0x00($sp)
    sw      $t0, 0x04($sp)
    sw      $t1, 0x08($sp)
    sw      $t2, 0x0C($sp)
    sw      $t3, 0x10($sp)
    sw      $t4, 0x14($sp)
    sw      $t5, 0x18($sp)
    sw      $t6, 0x1C($sp)
    sw      $t7, 0x20($sp)
    sw      $t8, 0x24($sp)
    sw      $t9, 0x28($sp)
    jal     queueCourseResourceSectorReads
     addiu  $t1, $t0, 0x20
    sw      $ra, 0x00($sp)
    lw      $t0, 0x04($sp)
    lw      $t1, 0x08($sp)
    lw      $t2, 0x0C($sp)
    lw      $t3, 0x10($sp)
    lw      $t4, 0x14($sp)
    lw      $t5, 0x18($sp)
    lw      $t6, 0x1C($sp)
    lw      $t7, 0x20($sp)
    lw      $t8, 0x24($sp)
    lw      $t9, 0x28($sp)
    addiu   $sp, $sp, 0x2C
    jr      $ra
endlabel func_800589F0
