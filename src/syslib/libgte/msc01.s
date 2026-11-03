# Handwritten assembly reference:
# https://github.com/hansbonini/psx_tomba/blob/b0aab7ef43785b257f66fb007df36b222e1a3665/src/psyq/libgte/msc01.s
.include "macro.inc"
.include "gte_macros.inc"

.set noat
.set noreorder

.section .text, "ax"

glabel SquareRoot0
      mtc2       $a0, $30
      nop
      nop
      mfc2       $v0, $31
      addiu      $at, $zero, 0x20
      beq        $v0, $at, .L800BA934
       nop
      andi       $t0, $v0, 0x1
      addiu      $at, $zero, -0x2
      and        $t2, $v0, $at
      addiu      $t1, $zero, 0x1F
      sub        $t1, $t1, $t2
      sra        $t1, $t1, 1
      addi       $t3, $t2, -0x18
      bltz       $t3, .L800BA900
       nop
      sllv       $t4, $a0, $t3
      b          .L800BA90C
  .L800BA900:
       addiu     $t3, $zero, 0x18
      sub        $t3, $t3, $t2
      srav       $t4, $a0, $t3
  .L800BA90C:
      addi       $t4, $t4, -0x40
      sll        $t4, $t4, 1
      lui        $t5, %hi(D_801355FC)
      addu       $t5, $t5, $t4
      lh         $t5, %lo(D_801355FC)($t5)
      nop
      sllv       $t5, $t5, $t1
      srl        $v0, $t5, 12
      jr         $ra
       nop
  .L800BA934:
      jr         $ra
       addiu     $v0, $zero, 0x0
endlabel SquareRoot0
      nop
      nop
      nop
