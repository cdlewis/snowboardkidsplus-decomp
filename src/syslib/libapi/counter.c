/* Adapted from PsyZ decomp; see THIRD_PARTY.md and licenses/psyz-decomp.txt. */
#include "common.h"
#include "counter_private.h"

#define COUNTER_DATA __attribute__((section(".data.counter"), aligned(4)))
volatile s32 *gCounterInterruptStatus COUNTER_DATA = (volatile s32 *)0x1F801070;
volatile RootCounterRegisters *gRootCounters COUNTER_DATA =
    (volatile RootCounterRegisters *)0x1F801100;
s32 gCounterInterruptMasks[4] COUNTER_DATA = {0x10, 0x20, 0x40, 1};

s32 SetRCnt(s32 spec, s16 target, s32 flags) {
    s32 i = spec & 0xFFFF;
    s32 final_mode = 0x48;
    if (i >= 3) {
        return 0;
    }
    gRootCounters[i].mode = 0;
    gRootCounters[i].target = target;

    if (i < 2u) {
        if (flags & 0x10) {
            final_mode = 0x49;
        }
        if (!(flags & 1)) {
            final_mode |= 0x100;
        }
    } else if (i == 2u) { // nb: `else` is redundant here
        if (!(flags & 1)) {
            final_mode = 0x248;
        }
    }
    if ((flags & 0x1000) != 0) {
        final_mode |= 0x10;
    }

    gRootCounters[i].mode = final_mode;
    return 1;
}

s32 GetRCnt(s32 spec) {
    s32 i = spec & 0xFFFF;
    if (i >= 3) {
        return 0;
    }
    return gRootCounters[i].rootCounter;
}

s32 StartRCnt(s32 spec) {
    s32 i = spec & 0xFFFF;
    gCounterInterruptStatus[1] |= gCounterInterruptMasks[i];
    return i < 3;
}

#ifdef NON_MATCHING
/* Best observed instruction-sequence similarity: 76.92% (10/13 words). */
s32 StopRCnt(s32 spec) {
    s32 i = spec & 0xFFFF;
    gCounterInterruptStatus[1] &= ~gCounterInterruptMasks[i];
    return 1;
}
#else
INCLUDE_ASM("asm/nonmatchings/syslib/libapi/counter", StopRCnt);
#endif

s32 ResetRCnt(s32 spec) {
    s32 i = spec & 0xFFFF;
    if (i >= 3) {
        return 0;
    }
    gRootCounters[i].rootCounter = 0;
    return 1;
}
