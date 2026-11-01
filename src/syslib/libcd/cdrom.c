#include "common.h"

extern u8 *StRingAddr;
extern s32 StRingSize;
extern void StClearRing(void);

void StSetRing(u8 *addr, s32 size) {
    StRingAddr = addr;
    StRingSize = size;
    StClearRing();
}
