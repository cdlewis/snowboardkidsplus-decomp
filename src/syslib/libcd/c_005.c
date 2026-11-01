#include "common.h"
extern void StSetMask(u32 mask, u32 start, u32 end);
extern s32 StEmu_Addr, StRgb24, CChannel, StCHANNEL, Stframe_no;
extern s16 Stsector_offset;
extern void (*StFunc1)(void), (*StFunc2)(void);
void StSetStream(u32 mode, u32 start, u32 end, void (*func1)(void), void (*func2)(void)) {
    StSetMask(1, start, end);
    StEmu_Addr = 0;
    StFunc1 = func1;
    StRgb24 = mode & 1;
    CChannel = 0;
    StCHANNEL = 0;
    Stsector_offset = 0;
    Stframe_no = 0;
    StFunc2 = func2;
}
