#include "common.h"
typedef struct {
    u32 status;
    u8 data[28];
} RingSlot;
extern u8 *StRingAddr;
void init_ring_status(s32 start, u32 count) {
    u32 i = 0;
    if (count == 0)
        return;
loop:
    {
        RingSlot *slot = (RingSlot *)(StRingAddr + ((i + start) * sizeof(RingSlot)));
        slot->status = 0;
    }
    i++;
    if (i < count)
        goto loop;
}
