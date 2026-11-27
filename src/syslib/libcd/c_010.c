#include "common.h"

/* Unnamed register pointers retained by this SDK object without references. */
volatile u8 *streamSetMaskCdRegisterAddresses[4]
    __attribute__((section(".data.stream_set_mask_registers"), aligned(4))) = {
    (volatile u8 *)0x1F801800,
    (volatile u8 *)0x1F801801,
    (volatile u8 *)0x1F801802,
    (volatile u8 *)0x1F801803,
};

/* Streaming state is owned by the CD streaming data object. */
extern s32 StSTART_FLAG, StStartFrame, StEndFrame;

void StSetMask(u32 mask, u32 start, u32 end) {
    StSTART_FLAG = mask;
    StStartFrame = start;
    StEndFrame = end;
}
