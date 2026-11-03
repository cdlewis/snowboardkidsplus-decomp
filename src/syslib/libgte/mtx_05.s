# Handwritten assembly reference:
# https://github.com/hansbonini/psx_tomba/blob/b0aab7ef43785b257f66fb007df36b222e1a3665/src/psyq/libgte/mtx_000.s
.include "macro.inc"
.include "gte_macros.inc"

.set noat
.set noreorder

.section .text, "ax"

glabel ApplyMatrix
      lw         $t0, 0x0($a0)
      lw         $t1, 0x4($a0)
      lw         $t2, 0x8($a0)
      lw         $t3, 0xC($a0)
      lw         $t4, 0x10($a0)
      ctc2       $t0, $0
      ctc2       $t1, $1
      ctc2       $t2, $2
      ctc2       $t3, $3
      ctc2       $t4, $4
      lwc2       $0, 0x0($a1)
      lwc2       $1, 0x4($a1)
      nop
      mvmva      1, 0, 0, 3, 0
      swc2       $25, 0x0($a2)
      swc2       $26, 0x4($a2)
      swc2       $27, 0x8($a2)
      addu       $v0, $a2, $zero
      jr         $ra
       nop
endlabel ApplyMatrix
