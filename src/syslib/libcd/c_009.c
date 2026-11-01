#include "common.h"

#ifdef NON_MATCHING
extern u8 *StRingAddr;
extern s32 StRingSize, StRingIdx3, StEndFrame;

/* Best observed instruction-sequence similarity: 91.67% (44/48 words,
 * -mno-split-addresses; target includes two padding words). */
u32 StGetNext(u32 **addr, u32 **header) {
    volatile u16 *slot = (u16 *)(StRingAddr + (StRingIdx3 << 5));

    if (slot[0] == 1) {
        StRingIdx3 = 0;
        if (StEndFrame != 0) slot[0] = 0;
        slot = (u16 *)(StRingAddr + (StRingIdx3 << 5));
    }
    if (slot[0] != 2) return 1;
    slot[0] = 4;
    *addr = (u32 *)(StRingAddr + (StRingSize << 5) + ((StRingIdx3 * 63) << 5));
    *header = (u32 *)slot;
    return 0;
}
#else
INCLUDE_ASM("asm/nonmatchings/syslib/libcd/c_009", StGetNext);
#endif
