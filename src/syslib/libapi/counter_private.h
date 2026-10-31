#ifndef COUNTER_PRIVATE_H
#define COUNTER_PRIVATE_H

#include "psx/libapi.h"

/* Each MMIO counter occupies 16 bytes; only its low halfwords are used. */
typedef struct RootCounterRegisters {
    u16 rootCounter;
    s16 pad02;
    s16 mode;
    s16 pad06;
    s16 target;
    s16 pad0A;
    s32 pad0C;
} RootCounterRegisters;

#endif
