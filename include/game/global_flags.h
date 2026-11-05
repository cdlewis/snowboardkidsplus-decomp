#pragma once

#include "common.h"

/* Verified prefix only; the remaining state layout is unknown. */
typedef struct GlobalFlagStatePrefix {
    u8 unk_000[0x3AC];
    u32 flags;
    u32 masks[10];
    u32 combinedMask;
    u32 unk_3DC;
    u32 unk_3E0;
    u32 unk_3E4;
} GlobalFlagStatePrefix;

extern GlobalFlagStatePrefix D_8013080C;
extern GlobalFlagStatePrefix D_8013101C;
extern u32 D_801313C8[];

u32 getGlobalFlagMask(s16 bitIndex);

void setPairedStateIndexedFlag(s16 index);

void rebuildCombinedStateMask(GlobalFlagStatePrefix *state);

void copyGlobalFlagState(GlobalFlagStatePrefix *source, GlobalFlagStatePrefix *destination);
