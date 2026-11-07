#pragma once

#include "common.h"
#include "game/race_time.h"

typedef struct SaveRecord {
    u8 unk_000[0x14C];
    RaceTime timeRecords[10][5];
    u8 unk_214[0x15E];
    u8 timeRecordCharacters[10][5];
    u8 unk_3A4[8];
    u32 flags_3AC;
    u32 flags_3B0[10];
    u32 flags_3D8;
    u8 unk_3DC[0x2C];
} SaveRecord;

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

#ifdef GLOBAL_FLAGS_GP_ADDRESSING
extern u8 D_8013AFD8;
extern u8 D_8013AFD9;
#else
extern u8 D_8013AFD8 __attribute__((section(".data")));
extern u8 D_8013AFD9 __attribute__((section(".data")));
#endif
/* GP-relative view of the shared task transition flag at D_8013B55E. */
extern u8 taskTransitionFlagGp;

extern GlobalFlagStatePrefix D_8013080C;
extern GlobalFlagStatePrefix D_8013101C;
extern u32 D_801313C8[];
extern GlobalFlagStatePrefix *D_8013B484[2];

u32 getGlobalFlagMask(s16 bitIndex);

void updatePairedStateFlag(s16 index);

void setPairedStateIndexedFlag(s16 index);

void rebuildCombinedStateMask(GlobalFlagStatePrefix *state);

void copyGlobalFlagState(GlobalFlagStatePrefix *source, GlobalFlagStatePrefix *destination);

s32 getSaveRecordFlagGroupIndex(s16 recordIndex, s16 groupIndex);

s16 getSaveRecordMaskTier(s16 recordIndex, s16 maskIndex);

void setSaveRecordMaskTier(s16 recordIndex, s16 maskIndex, s16 tier);

s16 getStateMaskTier(s16 stateIndex, s16 maskIndex);

s32 getStateMaskPair(s16 stateIndex, s16 maskIndex, s16 pairIndex);

void setStateGridFlag(GlobalFlagStatePrefix *state, s16 row, s16 column);
s16 getSaveRecordFlagPair(s16 recordIndex, s16 maskIndex, s16 pairIndex);

void setSaveRecordFlagPair(s16 recordIndex, s16 maskIndex, s16 pairIndex, s16 value);

s32 hasStateGridFlag(GlobalFlagStatePrefix *state, s16 row, s16 column);

s32 getPlayerRecordFlagMode(s16 index);

extern s16 D_8013AFDE __attribute__((section(".data")));
extern u8 D_8013AFE1 __attribute__((section(".data")));

#ifdef SAVE_OVERRIDE_ABSOLUTE_ADDRESSING
extern u8 D_8013B11A __attribute__((section(".data")));
#else
extern u8 D_8013B11A;
#endif

void copySaveRecordAndMergeFlags(void);
void applySaveRecordUnlockOverrides(s16 index);
