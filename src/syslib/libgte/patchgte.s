# Handwritten assembly reference:
# https://github.com/hansbonini/psx_tomba/blob/b0aab7ef43785b257f66fb007df36b222e1a3665/src/psyq/libgte/patchgte.s
# https://github.com/ser-pounce/rood-reverse/blob/8bd7908d878824d6cfa02ce65e887dfaf586f11f/src/SLUS_010.40/libgte/PATCHGTE.s
.include "macro.inc"
.include "gte_macros.inc"

.set noat
.set noreorder

.section .text, "ax"

glabel _patch_gte
      lui        $at, %hi(SAVERA2)
      sw         $ra, %lo(SAVERA2)($at)
      jal        EnterCriticalSection
       nop
      addiu      $t1, $zero, 0x56
      addiu      $t2, $zero, 0xB0
      jalr       $t2
       nop
      lw         $v0, 0x18($v0)
      nop
      addiu      $v0, $v0, 0x28
      addu       $t7, $v0, $zero
      lui        $t2, %hi(func_800BF0D4)
      addiu      $t2, $t2, %lo(func_800BF0D4)
      lui        $t1, %hi(gteExceptionPatchReplacement)
      addiu      $t1, $t1, %lo(gteExceptionPatchReplacement)
  .L800BF068:
      lw         $v1, 0x0($t2)
      lw         $t3, 0x0($v0)
      addiu      $t2, $t2, 0x4
      bne        $v1, $t3, .L800BF0B0
       addiu     $v0, $v0, 0x4
      bne        $t2, $t1, .L800BF068
       nop
      addu       $v0, $t7, $zero
      lui        $t2, %hi(gteExceptionPatchReplacement)
      addiu      $t2, $t2, %lo(gteExceptionPatchReplacement)
      lui        $t1, %hi(gteExceptionPatchEnd)
      addiu      $t1, $t1, %lo(gteExceptionPatchEnd)
  .L800BF098:
      lw         $v1, 0x0($t2)
      nop
      sw         $v1, 0x0($v0)
      addiu      $t2, $t2, 0x4
      bne        $t2, $t1, .L800BF098
       addiu     $v0, $v0, 0x4
  .L800BF0B0:
      jal        FlushCache
       nop
      jal        ExitCriticalSection
       nop
      lui        $ra, %hi(SAVERA2)
      lw         $ra, %lo(SAVERA2)($ra)
      nop
      jr         $ra
       nop
endlabel _patch_gte
