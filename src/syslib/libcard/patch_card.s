# Handwritten assembly reference:
# https://github.com/hansbonini/psx_tomba/blob/b0aab7ef43785b257f66fb007df36b222e1a3665/src/psyq/libcard/patch.s
.include "macro.inc"
.include "gte_macros.inc"

.set noat
.set noreorder

.section .text, "ax"

glabel _patch_card
      lui        $at, %hi(D_80141638)
      sw         $ra, %lo(D_80141638)($at)
      jal        EnterCriticalSection
       nop
      addiu      $t1, $zero, 0x56
      addiu      $t2, $zero, 0xB0
      jalr       $t2
       nop
      lw         $v0, 0x18($v0)
      nop
      lw         $v1, 0x70($v0)
      nop
      andi       $t1, $v1, 0xFFFF
      sll        $t1, $t1, 16
      lw         $v1, 0x74($v0)
      nop
      andi       $t2, $v1, 0xFFFF
      addu       $v1, $t1, $t2
      addiu      $v0, $v1, 0x28
      lui        $t2, %hi(func_800D6A58)
      addiu      $t2, $t2, %lo(func_800D6A58)
      lui        $t1, %hi(func_800D6A58 + 0x14)
      addiu      $t1, $t1, %lo(func_800D6A58 + 0x14)
  .L800D6ADC:
      lw         $v1, 0x0($t2)
      nop
      sw         $v1, 0x0($v0)
      addiu      $t2, $t2, 0x4
      bne        $t2, $t1, .L800D6ADC
       addiu     $v0, $v0, 0x4
      lui        $at, (0x10000 >> 16)
      jal        FlushCache
       sw        $v0, -0x2004($at)
      lui        $ra, %hi(D_80141638)
      lw         $ra, %lo(D_80141638)($ra)
      nop
      jr         $ra
       nop
endlabel _patch_card
