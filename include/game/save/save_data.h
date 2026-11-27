#pragma once

#include "common.h"
#include "game/race/ui/race_time.h"

enum { SAVE_RECORD_COMBINED_MASK_INDEX = 10 };

typedef struct SaveRecord {
    u8 resourceTextureSelections[9];
    u8 unk_009[3];
    u32 bestScore[11];
    u8 bestPlace[11];
    u8 unk_043;
    RaceTime lapRecords[11][5];
    RaceTime bestLapSplitTimes[11];
    RaceTime timeRecords[11][5];
    u16 statsRecordValues[11][5];
    u8 statsRecordIcons[11][5];
    u8 lapRecordIcons[11][5];
    u8 countRecordValues[11][5];
    u8 countRecordIcons[11][5];
    u8 timeRecordCharacters[11][5];
    u8 unk_3A9[3];
    u32 flags;
    /* Ten character masks followed by their bitwise union. */
    u32 masks[SAVE_RECORD_COMBINED_MASK_INDEX + 1];
    u32 boardUnlockFlagsLow;
    u32 boardUnlockFlagsHigh;
    u32 money;
    u32 unk_3E8;
    u8 boardSelections[10];
    u8 paletteSelections[10];
    u16 unlockNotificationFlags;
    u16 unk_402;
    u32 checksum;
} SaveRecord;

extern u8 D_8013AFD8;
/* Zero disables prerecorded races; other values index playback entries. */
extern u8 gControllerPlaybackIndex;
/* GP-relative view of the shared task transition flag at D_8013B55E. */
extern u8 taskTransitionFlagGp;

extern SaveRecord gPlayerSaveRecords[2];
extern SaveRecord gMergedSaveRecord;
extern SaveRecord *gWorkingSaveRecords[2];

void initializeSaveRecordDefaults(SaveRecord *record);

u32 getGlobalFlagMask(s16 bitIndex);

void updatePairedStateFlag(s16 index);

void setPairedStateIndexedFlag(s16 index);

void rebuildCombinedStateMask(SaveRecord *state);

void copyGlobalFlagState(SaveRecord *source, SaveRecord *destination);
void mergeSaveRecordFlags(SaveRecord *source, SaveRecord *destination);

s32 getSaveRecordFlagGroupIndex(s16 recordIndex, s16 groupIndex);

s16 getSaveRecordMaskTier(s16 recordIndex, s16 maskIndex);

void setSaveRecordMaskTier(s16 recordIndex, s16 maskIndex, s16 tier);

s16 getStateMaskTier(s16 stateIndex, s16 maskIndex);

s32 getStateMaskPair(s16 stateIndex, s16 maskIndex, s16 pairIndex);

void setStateGridFlag(SaveRecord *state, s16 row, s16 column);
s16 getSaveRecordFlagPair(s16 recordIndex, s16 maskIndex, s16 pairIndex);

void setSaveRecordFlagPair(s16 recordIndex, s16 maskIndex, s16 pairIndex, s16 value);

s16 updateRaceUnlockProgress(s16 characterId, s16 finishPosition);

s32 hasStateGridFlag(SaveRecord *state, s16 row, s16 column);

s32 getPlayerRecordFlagMode(s16 index);

extern s16 D_8013AFDE;

extern u8 gSaveRecordUnlockOverrideFlags;

void copySaveRecordAndMergeFlags(void);
void applyLoadedSaveRecordSettings(SaveRecord *source, s16 index);
void applySaveRecordUnlockOverrides(s16 index);

typedef char SaveRecordSizeCheck[(sizeof(SaveRecord) == 0x408) ? 1 : -1];
