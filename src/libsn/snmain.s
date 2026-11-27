# SN Systems startup. The Silent Hill decompilation documents the
# corresponding handwritten routine from snmain.obj:
# https://github.com/shdecompilations/silent-hill-decomp/blob/eb96675c5cb51fe87a66db1df20490c30e5ded6d/src/main/libsn/snmain.s#L6-L60

.include "macro.inc"

.set noreorder
.set noat
.text

glabel __SN_ENTRY_POINT
    lui     $v0, %hi(snStartupSavedReturnAddress)
    addiu   $v0, $v0, %lo(snStartupSavedReturnAddress)
    lui     $v1, %hi(sdk_libapi_chclrpad_bss_BSS_END)
    addiu   $v1, $v1, %lo(sdk_libapi_chclrpad_bss_BSS_END)
.LclearBss:
    sw      $zero, 0($v0)
    addiu   $v0, $v0, 4
    sltu    $at, $v0, $v1
    bne     $at, $zero, .LclearBss
     nop

    addiu   $v0, $zero, 4
    nop
    nop
    nop
    nop
    lui     $a0, %hi(snStartupStackTops)
    addiu   $a0, $a0, %lo(snStartupStackTops)
    addu    $a0, $a0, $v0
    lw      $v0, 0($a0)
    lui     $t0, 0x8000
    or      $sp, $v0, $t0

    lui     $a0, %hi(sdk_libapi_chclrpad_bss_BSS_END)
    addiu   $a0, $a0, %lo(sdk_libapi_chclrpad_bss_BSS_END)
    sll     $a0, $a0, 3
    srl     $a0, $a0, 3
    lui     $v1, %hi(_stacksize)
    lw      $v1, %lo(_stacksize)($v1)
    nop
    subu    $a1, $v0, $v1
    subu    $a1, $a1, $a0
    or      $a0, $a0, $t0

    lui     $at, %hi(snStartupSavedReturnAddress)
    sw      $ra, %lo(snStartupSavedReturnAddress)($at)
    lui     $gp, %hi(_gp)
    addiu   $gp, $gp, %lo(_gp)
    addu    $fp, $sp, $zero
    jal     InitHeap
     addi   $a0, $a0, 4

    lui     $ra, %hi(snStartupSavedReturnAddress)
    lw      $ra, %lo(snStartupSavedReturnAddress)($ra)
    nop
    jal     runMovieAndGameLoop
     nop
    break   0, 1
endlabel __SN_ENTRY_POINT

dlabel snStartupStackTops
    .word   0x00200000, 0x00200000, 0x00200000, 0x00200000
enddlabel snStartupStackTops
