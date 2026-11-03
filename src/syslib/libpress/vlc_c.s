# Handwritten assembly reference:
# https://github.com/ser-pounce/rood-reverse/blob/8bd7908d878824d6cfa02ce65e887dfaf586f11f/src/TITLE/TITLE.PRG/libpress/VLC_C.s
.include "macro.inc"

.set noat
.set noreorder

.section .text, "ax"

glabel DecDCTvlcSize2
    lui        $t0, %hi(decodeSize)
    addiu      $t0, $t0, %lo(decodeSize)
    addi       $at, $a0, -0x1
    blez       $at, .L800D0968
     lw        $v0, 0x0($t0)
    sll        $at, $a0, 1
    jr         $ra
     sw        $at, 0x0($t0)
.L800D0968:
    lui        $at, (0xFFFFFF >> 16)
    ori        $at, $at, (0xFFFFFF & 0xFFFF)
    jr         $ra
     sw        $at, 0x0($t0)
endlabel DecDCTvlcSize2

glabel DecDCTvlc2
    lui        $t0, %hi(decodeSize)
    addiu      $t0, $t0, %lo(decodeSize)
    addi       $a2, $a2, 0x800
    lui        $at, (0x10000 >> 16)
    add        $a3, $a2, $at
    bnez       $a0, .L800D09CC
     lw        $t1, 0x0($t0)
    lui        $t0, %hi(vlcSavedRegisters)
    addiu      $t0, $t0, %lo(vlcSavedRegisters)
    lw         $a0, 0x0($t0)
    lw         $a1, 0x4($t0)
    lw         $v0, 0x8($t0)
    lw         $v1, 0xC($t0)
    lw         $t4, 0x10($t0)
    lw         $t5, 0x14($t0)
    lw         $t7, 0x18($t0)
    lw         $t8, 0x1C($t0)
    lw         $t9, 0x20($t0)
    add        $t1, $t1, $t1
    b          .L800D0B58
     add       $t6, $a1, $t1
.L800D09CC:
    add        $t5, $zero, $zero
    add        $t7, $zero, $zero
    add        $t8, $zero, $zero
    add        $t9, $zero, $zero
    add        $t1, $t1, $t1
    add        $t6, $a1, $t1
    lw         $t1, 0x0($a0)
    lhu        $t4, 0x4($a0)
    lhu        $t2, 0x6($a0)
    lhu        $v0, 0x8($a0)
    lhu        $v1, 0xA($a0)
    addi       $t2, $t2, -0x3
    bltz       $t2, .L800D0A08
     sll       $t4, $t4, 10
    addi       $t5, $zero, 0x1
.L800D0A08:
    addi       $a0, $a0, 0xC
    sll        $v0, $v0, 16
    or         $v0, $v0, $v1
    or         $v1, $zero, $zero
    sw         $t1, 0x0($a1)
    andi       $t1, $t1, 0xFFFF
    sll        $t1, $t1, 2
    addiu      $t1, $t1, 0x4
    add        $t1, $t1, $a1
    lui        $t0, %hi(vlcOutputEnd)
    addiu      $t0, $t0, %lo(vlcOutputEnd)
    sw         $t1, 0x0($t0)
    addi       $a1, $a1, 0x2
.L800D0A3C:
    beqz       $t5, .L800D0B14
     srl       $t0, $v0, 22
    xori       $at, $t0, 0x3FF
    beqz       $at, .L800D0C60
     addi      $a1, $a1, 0x2
    addi       $at, $t5, -0x3
    bltz       $at, .L800D0A60
     addi      $at, $a2, -0x400
    addi       $at, $at, -0x400
.L800D0A60:
    srl        $t0, $v0, 24
    sll        $t0, $t0, 2
    add        $t0, $t0, $at
    lhu        $t1, 0x0($t0)
    lhu        $t2, 0x2($t0)
    and        $t0, $zero, $zero
    beqz       $t2, .L800D0AA4
     sllv      $v0, $v0, $t1
    addi       $at, $zero, 0x20
    sub        $at, $at, $t2
    srlv       $t0, $v0, $at
    bltz       $v0, .L800D0AA0
     sllv      $v0, $v0, $t2
    addi       $t3, $zero, -0x1
    srlv       $t3, $t3, $at
    sub        $t0, $t0, $t3
.L800D0AA0:
    add        $v1, $v1, $t2
.L800D0AA4:
    add        $v1, $v1, $t1
    andi       $at, $v1, 0x10
    beqz       $at, .L800D0AC4
     andi      $v1, $v1, 0xF
    lhu        $t1, 0x0($a0)
    addi       $a0, $a0, 0x2
    sllv       $t1, $t1, $v1
    or         $v0, $v0, $t1
.L800D0AC4:
    addi       $at, $t5, -0x2
    bgtz       $at, .L800D0AEC
     add       $t1, $t9, $t0
    beqz       $at, .L800D0AE4
     add       $t1, $t8, $t0
    add        $t1, $t7, $t0
    b          .L800D0AF0
     add       $t7, $t7, $t0
.L800D0AE4:
    b          .L800D0AF0
     add       $t8, $t8, $t0
.L800D0AEC:
    add        $t9, $t9, $t0
.L800D0AF0:
    sll        $t1, $t1, 2
    andi       $t1, $t1, 0x3FF
    or         $t1, $t4, $t1
    addi       $t5, $t5, 0x1
    addi       $at, $t5, -0x7
    bnez       $at, .L800D0B4C
     sh        $t1, 0x0($a1)
    b          .L800D0B4C
     addi      $t5, $t5, -0x6
.L800D0B14:
    xori       $at, $t0, 0x1FF
    beqz       $at, .L800D0C60
     addi      $a1, $a1, 0x2
    sll        $v0, $v0, 10
    addi       $v1, $v1, 0xA
    andi       $at, $v1, 0x10
    beqz       $at, .L800D0B44
     andi      $v1, $v1, 0xF
    lhu        $t1, 0x0($a0)
    addi       $a0, $a0, 0x2
    sllv       $t1, $t1, $v1
    or         $v0, $v0, $t1
.L800D0B44:
    or         $t0, $t4, $t0
    sh         $t0, 0x0($a1)
.L800D0B4C:
    subu       $at, $a1, $t6
    bgez       $at, .L800D0CA4
     addi      $a1, $a1, 0x2
.L800D0B58:
    srl        $t0, $v0, 19
    sll        $t0, $t0, 3
    add        $t0, $t0, $a2
    lw         $t1, 0x0($t0)
    nop
    bnez       $t1, .L800D0BB4
     andi      $at, $t1, 0xFF
    sll        $v0, $v0, 8
    addi       $v1, $v1, 0x8
    andi       $at, $v1, 0x10
    beqz       $at, .L800D0B98
     andi      $v1, $v1, 0xF
    lhu        $t0, 0x0($a0)
    addi       $a0, $a0, 0x2
    sllv       $t0, $t0, $v1
    or         $v0, $v0, $t0
.L800D0B98:
    srl        $t0, $v0, 23
    sll        $t0, $t0, 2
    add        $t0, $t0, $a3
    lw         $t1, 0x0($t0)
    add        $t3, $zero, $zero
    b          .L800D0BB8
     andi      $at, $t1, 0xFF
.L800D0BB4:
    lw         $t3, 0x4($t0)
.L800D0BB8:
    sllv       $v0, $v0, $at
    add        $v1, $v1, $at
    andi       $at, $v1, 0x10
    beqz       $at, .L800D0BDC
     andi      $v1, $v1, 0xF
    lhu        $t0, 0x0($a0)
    addi       $a0, $a0, 0x2
    sllv       $t0, $t0, $v1
    or         $v0, $v0, $t0
.L800D0BDC:
    srl        $t1, $t1, 16
    xori       $at, $t1, 0x7C1F
    beqz       $at, .L800D0C3C
     xori      $at, $t1, 0xFE00
    beqz       $at, .L800D0A3C
     sh        $t1, 0x0($a1)
    beqz       $t3, .L800D0B58
     addi      $a1, $a1, 0x2
    andi       $t2, $t3, 0xFFFF
    xori       $at, $t2, 0x7C1F
    beqz       $at, .L800D0C3C
     xori      $at, $t2, 0xFE00
    beqz       $at, .L800D0A3C
     sh        $t2, 0x0($a1)
    srl        $t2, $t3, 16
    beqz       $t2, .L800D0B58
     addi      $a1, $a1, 0x2
    xori       $at, $t2, 0x7C1F
    beqz       $at, .L800D0C3C
     xori      $at, $t2, 0xFE00
    beqz       $at, .L800D0A3C
     sh        $t2, 0x0($a1)
    b          .L800D0B58
     addi      $a1, $a1, 0x2
.L800D0C3C:
    srl        $t0, $v0, 16
    sh         $t0, 0x0($a1)
    addi       $a1, $a1, 0x2
    lhu        $t0, 0x0($a0)
    addi       $a0, $a0, 0x2
    sll        $v0, $v0, 16
    sllv       $t0, $t0, $v1
    b          .L800D0B58
     or        $v0, $v0, $t0
.L800D0C60:
    lui        $t0, %hi(vlcOutputEnd)
    addiu      $t0, $t0, %lo(vlcOutputEnd)
    lw         $t1, 0x0($t0)
    ori        $t0, $zero, 0xFE00
.L800D0C70:
    subu       $at, $a1, $t1
    bgez       $at, .L800D0C88
     nop
    sh         $t0, 0x0($a1)
    b          .L800D0C70
     addi      $a1, $a1, 0x2
.L800D0C88:
    mfc0       $t1, $12
    nop
    lui        $at, (0x20000 >> 16)
    or         $t1, $t1, $at
    mtc0       $t1, $12
    jr         $ra
     add       $v0, $zero, $zero
.L800D0CA4:
    lui        $t0, %hi(vlcSavedRegisters)
    addiu      $t0, $t0, %lo(vlcSavedRegisters)
    sw         $a0, 0x0($t0)
    sw         $a1, 0x4($t0)
    sw         $v0, 0x8($t0)
    sw         $v1, 0xC($t0)
    sw         $t4, 0x10($t0)
    sw         $t5, 0x14($t0)
    sw         $t7, 0x18($t0)
    sw         $t8, 0x1C($t0)
    sw         $t9, 0x20($t0)
    jr         $ra
     addi      $v0, $zero, 0x1
endlabel DecDCTvlc2
