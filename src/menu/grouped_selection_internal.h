#pragma once

#include "game/engine/task.h"

typedef struct RegisteredPanelSlot {
    GameTask *task;
    s16 command;
    s16 flags;
} RegisteredPanelSlot;

typedef struct RecordFlagMenuEntries {
    u8 *entries;
    u8 unk_04;
    u8 unk_05;
    u8 count;
    u8 *textIndices;
    u32 unk_0C;
    u32 unk_10;
} RecordFlagMenuEntries;

typedef struct IndexedHalfwordRecord {
    s16 value;
    s16 unk_02;
    s16 unk_04;
} IndexedHalfwordRecord;

extern u8 gRecordFlagMenuItemBuffers[2][5];
extern u8 gRecordFlagMenuTextIndexBuffers[2][5];
extern u8 gDefaultBoardMenuItems[4];

void buildRecordFlagMenuEntries(s16 index, s16 mode, s16 group, u16 arg4, u16 arg5,
                                RecordFlagMenuEntries *out);
s16 findIndexedHalfwordRecord(const IndexedHalfwordRecord *records, const u8 *indices, s16 count, s16 value);
s16 getGroupedIndexBase(s16 index);
s16 getGroupedIndexOffset(s16 index);
s16 getGroupedIndexOffsetPlus216(s16 index);
s16 getTaskResourceAdjustedGroupedIndex(s16 value, GameTask *task, s16 index);
s16 getTaskResourceGroupedIndex(GameTask *task, s16 index);
void adjustGroupedIndexForMode(s16 mode, u8 *index);
s32 getPlayerRecordFlagMode(s16 index);
s16 mapGroupedValueToTableIndex(s16 value);
void initializeModeDependentDualResourceTask(void);
