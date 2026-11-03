# Handwritten assembly reference:
# https://github.com/hansbonini/psx_tomba/blob/b0aab7ef43785b257f66fb007df36b222e1a3665/src/psyq/libcard/patch.s
.include "macro.inc"
.include "gte_macros.inc"

.set noat
.set noreorder

.section .text, "ax"

glabel _patch_card2
      lui        $at, %hi(D_80141638)
      sw         $ra, %lo(D_80141638)($at)
      jal        EnterCriticalSection
       nop
      addiu      $t1, $zero, 0x57
      addiu      $t2, $zero, 0xB0
      jalr       $t2
       nop
      lw         $v0, 0x16C($v0)
      nop
      lw         $v1, 0x9C8($v0)
      lui        $t2, %hi(func_800D6A58 + 0x14)
      addiu      $t2, $t2, %lo(func_800D6A58 + 0x14)
      lui        $t1, %hi(_patch_card)
      addiu      $t1, $t1, %lo(_patch_card)
  .L800D6B50:
      lw         $t0, 0x0($t2)
      nop
      sw         $t0, 0x9C8($v0)
      addiu      $t2, $t2, 0x4
      bne        $t2, $t1, .L800D6B50
       addiu     $v0, $v0, 0x4
      jal        FlushCache
       nop
      lui        $ra, %hi(D_80141638)
      lw         $ra, %lo(D_80141638)($ra)
      nop
      jr         $ra
       nop
endlabel _patch_card2
