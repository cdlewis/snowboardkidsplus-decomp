#include "common.h"
typedef struct {
    u16 id, type, sector_count, nsectors;
    u8 remaining[24];
} StHEADER;
extern u8 *StRingAddr;
extern s32 StRingSize, StRingIdx3;
u32 StFreeRing(u32 *base) {
    int slot, i;
    s16 nsectors;
    StHEADER *frame;
    u16 *sector;
    slot = (base - (u32 *)(StRingAddr + (StRingSize * sizeof(StHEADER)))) / 504;
    frame = (StHEADER *)(StRingAddr + (slot * sizeof(StHEADER)));
    nsectors = frame->nsectors;
    if ((s16)frame->id != 4)
        return 1;
    i = 0;
    if (nsectors > 0) do {
        sector = (u16 *)(StRingAddr + ((i + slot) * sizeof(StHEADER)));
        i++;
        *sector = 0;
    } while (i < nsectors);
    StRingIdx3 = i + slot;
    return 0;
}
