#include "common.h"
#include "psx/libcd.h"

#ifdef NON_MATCHING
struct RingEntry {
    u16 id, unknown2, unknown4, unknown6;
    s32 frame;
    u8 unknownC[16];
    CdlLOC loc;
};
extern u8 *StRingAddr;
extern s32 StRingIdx1, StRingIdx2, next_frameid, StFinalSector;
extern CdlLOC previous_frame_location;
extern void (*StFunc1)(void);
/* Best observed instruction-sequence similarity: 85.71% (30/35 words). */
void data_ready_callback(void) {
    struct RingEntry *entry = (struct RingEntry *)(StRingAddr + (StRingIdx2 << 5));
    entry->id = 2;
    previous_frame_location = entry->loc;
    next_frameid = entry->frame;
    StRingIdx2 = StRingIdx1;
    if (StFunc1)
        StFunc1();
    StFinalSector = 0;
}
#else
INCLUDE_ASM("asm/nonmatchings/syslib/libcd/c_004", data_ready_callback);
#endif

extern s32 StMode;
extern CdlLOC previous_frame_location;
extern s32 next_frameid;
s32 StGetBackloc(CdlLOC *loc) {
    if (StMode) return -1;
    CdIntToPos(CdPosToInt(&previous_frame_location) + 1, loc);
    return next_frameid;
}
