#include "common.h"
extern s32 StRingIdx3, StRingIdx2, StRingIdx1, StFinalSector;
extern s32 StCdIntrFlag, Stframe_no;
extern s16 Stsector_offset;
extern s32 StRingSize;
extern void init_ring_status(s32 start, u32 count);
void StClearRing(void) {
    StRingIdx3 = 0;
    StRingIdx2 = 0;
    StRingIdx1 = 0;
    StFinalSector = 0;
    init_ring_status(0, StRingSize);
    StCdIntrFlag = 0;
    Stsector_offset = 0;
    Stframe_no = 0;
}
