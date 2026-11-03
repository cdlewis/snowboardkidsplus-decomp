# Handwritten assembly reference:
# https://github.com/hansbonini/psx_tomba/blob/b0aab7ef43785b257f66fb007df36b222e1a3665/src/psyq/libgte/msc00.s
# https://github.com/ser-pounce/rood-reverse/blob/8bd7908d878824d6cfa02ce65e887dfaf586f11f/src/SLUS_010.40/libgte/MSC00.s
.include "macro.inc"
.include "gte_macros.inc"

.set noat
.set noreorder

.section .text, "ax"

dlabel func_800BEF98
     .word 0x21097350
     .word 0x004242AD
enddlabel func_800BEF98

glabel InitGeom
      lui        $at, %hi(SAVERA)
      sw         $ra, %lo(SAVERA)($at)
      jal        _patch_gte
       nop
      lui        $ra, %hi(SAVERA)
      lw         $ra, %lo(SAVERA)($ra)
      nop
      mfc0       $v0, $12
      lui        $v1, (0x40000000 >> 16)
      or         $v0, $v0, $v1
      mtc0       $v0, $12
      nop
      addiu      $t0, $zero, 0x155
      ctc2       $t0, $29
      nop
      addiu      $t0, $zero, 0x100
      ctc2       $t0, $30
      nop
      addiu      $t0, $zero, 0x3E8
      ctc2       $t0, $26
      nop
      addiu      $t0, $zero, -0x1062
      ctc2       $t0, $27
      nop
      lui        $t0, (0x1400000 >> 16)
      ctc2       $t0, $28
      nop
      ctc2       $zero, $24
      ctc2       $zero, $25
      nop
      jr         $ra
       nop
endlabel InitGeom
      nop
      nop
