#pragma once

#include "common.h"
#include "game/race/ui/race_time.h"

typedef struct SaveRecord {
    u8 unk_000[0x44];
    RaceTime lapRecords[10][5];
    u8 unk_10C[0x40];
    RaceTime timeRecords[10][5];
    u8 unk_214[0x14];
    u16 statsRecordValues[10][5];
    u8 unk_28C[10];
    u8 statsRecordIcons[10][5];
    u8 unk_2C8[5];
    u8 lapRecordIcons[10][5];
    u8 unk_2FF[5];
    u8 countRecordValues[10][5];
    u8 unk_336[5];
    u8 countRecordIcons[10][5];
    u8 unk_36D[5];
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

typedef char SaveRecordSizeCheck[(sizeof(SaveRecord) == 0x408) ? 1 : -1];
typedef char GlobalFlagStatePrefixSizeCheck[(sizeof(GlobalFlagStatePrefix) == 0x3E8) ? 1 : -1];

typedef char SaveRecord_statsRecordValues_OffsetCheck[((unsigned int)&((SaveRecord *)0)->statsRecordValues == 0x228) ? 1 : -1];
typedef char SaveRecord_statsRecordIcons_OffsetCheck[((unsigned int)&((SaveRecord *)0)->statsRecordIcons == 0x296) ? 1 : -1];
typedef char SaveRecord_lapRecordIcons_OffsetCheck[((unsigned int)&((SaveRecord *)0)->lapRecordIcons == 0x2CD) ? 1 : -1];
typedef char SaveRecord_countRecordValues_OffsetCheck[((unsigned int)&((SaveRecord *)0)->countRecordValues == 0x304) ? 1 : -1];
typedef char SaveRecord_countRecordIcons_OffsetCheck[((unsigned int)&((SaveRecord *)0)->countRecordIcons == 0x33B) ? 1 : -1];
typedef char SaveRecord_timeRecordCharacters_OffsetCheck[((unsigned int)&((SaveRecord *)0)->timeRecordCharacters == 0x372) ? 1 : -1];
