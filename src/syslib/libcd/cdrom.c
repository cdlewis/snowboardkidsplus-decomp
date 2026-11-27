#include "common.h"

/* Retained by the SDK CDROM object even though StSetRing does not use them. */
volatile u8 *cdromRegisterAddresses[4] __attribute__((section(".data.cdrom_registers"))) = {
    (volatile u8 *)0x1F801800,
    (volatile u8 *)0x1F801801,
    (volatile u8 *)0x1F801802,
    (volatile u8 *)0x1F801803,
};

extern u8 *StRingAddr;
extern s32 StRingSize;
extern void StClearRing(void);

void StSetRing(u8 *addr, s32 size) {
    StRingAddr = addr;
    StRingSize = size;
    StClearRing();
}
