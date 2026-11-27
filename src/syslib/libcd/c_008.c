#include "common.h"

typedef struct {
    u32 status;
    u8 data[28];
} RingSlot;

/* Unnamed register pointers retained by this SDK object without references. */
volatile u8 *streamInitRingCdRegisterAddresses[4]
    __attribute__((section(".data.stream_init_ring_registers"), aligned(4))) = {
    (volatile u8 *)0x1F801800,
    (volatile u8 *)0x1F801801,
    (volatile u8 *)0x1F801802,
    (volatile u8 *)0x1F801803,
};

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
