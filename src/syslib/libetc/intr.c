#include "common.h"
#include "libetc_private.h"

/* SDK dispatch wrappers adapted from PsyZ decomp; see THIRD_PARTY.md. */

s32 ResetCallback(void) { return gInterruptDispatch->start(); }

DmaCallback InterruptCallback(s32 channel, DmaCallback cb) { return gInterruptDispatch->interrupt(channel, cb); }

DmaCallback DMACallback(s32 channel, DmaCallback cb) { return gInterruptDispatch->dma(channel, cb); }

DmaCallback VSyncCallback(DmaCallback cb) { return gInterruptDispatch->vsync(4, cb); }

DmaCallback VSyncCallbacks(s32 channel, DmaCallback cb) { return gInterruptDispatch->vsync(channel, cb); }

s32 StopCallback(void) { return gInterruptDispatch->stop(); }

s32 RestartCallback(void) { return gInterruptDispatch->restart(); }

s32 CheckCallback(void) { return gInterruptCallbackContext; }

s32 GetIntrMask(void) { return *gInterruptMaskRegister; }

s32 SetIntrMask(s32 mask) { s32 prev = *gInterruptMaskRegister; *gInterruptMaskRegister = mask; return prev; }

INCLUDE_ASM("asm/nonmatchings/syslib/libetc/intr", func_800B595C);

INCLUDE_ASM("asm/nonmatchings/syslib/libetc/intr", func_800B5A34);

INCLUDE_ASM("asm/nonmatchings/syslib/libetc/intr", func_800B5C04);

INCLUDE_ASM("asm/nonmatchings/syslib/libetc/intr", func_800B5D4C);

INCLUDE_ASM("asm/nonmatchings/syslib/libetc/intr", func_800B5DEC);

INCLUDE_ASM("asm/nonmatchings/syslib/libetc/intr", func_800B5E64);
