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

extern u8 D_8013AFD8 __attribute__((section(".data")));
extern u8 D_8013AFD9 __attribute__((section(".data")));

extern GlobalFlagStatePrefix D_8013080C;
extern GlobalFlagStatePrefix D_8013101C;
extern u32 D_801313C8[];
extern GlobalFlagStatePrefix *D_8013B484[];

u32 getGlobalFlagMask(s16 bitIndex);

void updatePairedStateFlag(s16 index);

void setPairedStateIndexedFlag(s16 index);

void rebuildCombinedStateMask(GlobalFlagStatePrefix *state);

void copyGlobalFlagState(GlobalFlagStatePrefix *source, GlobalFlagStatePrefix *destination);

s32 getSaveRecordFlagGroupIndex(s16 recordIndex, s16 groupIndex);

s16 getSaveRecordMaskTier(s16 recordIndex, s16 maskIndex);

s16 getStateMaskTier(s16 stateIndex, s16 maskIndex);

s32 getStateMaskPair(s16 stateIndex, s16 maskIndex, s16 pairIndex);

void setStateGridFlag(GlobalFlagStatePrefix *state, s16 row, s16 column);
s16 getSaveRecordFlagPair(s16 recordIndex, s16 maskIndex, s16 pairIndex);

s32 hasStateGridFlag(GlobalFlagStatePrefix *state, s16 row, s16 column);

s32 getPlayerRecordFlagMode(s16 index);
