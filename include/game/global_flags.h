#pragma once

#include "common.h"

/* Verified prefix only; the remaining state layout is unknown. */
typedef struct GlobalFlagStatePrefix {
    u8 unk_000[0x3AC];
    u32 flags;
} GlobalFlagStatePrefix;

extern GlobalFlagStatePrefix D_8013080C;
extern GlobalFlagStatePrefix D_8013101C;
extern u32 D_801313C8[];

u32 getGlobalFlagMask(s16 bitIndex);

void setPairedStateIndexedFlag(s16 index);
