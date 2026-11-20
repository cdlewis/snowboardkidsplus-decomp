#pragma once

#include "game/engine/task.h"

typedef struct TimedSequenceEntry {
    s32 unk00;
    s32 duration;
} TimedSequenceEntry;

typedef struct ValuePanelEntry {
    GameTask *task;
    s16 flags;
    s16 active;
} ValuePanelEntry;

extern const TimedSequenceEntry D_80123840[];
extern u8 D_8012D460[12];

void createInputTimedSpriteTask(s32 x, s32 y, const TimedSequenceEntry *entries, s32 count,
                                s32 controllers, u32 acceptMask, u32 cancelMask, s32 depth, s32 index);

s32 func_80070B64(s16 index, u8 *color);
void func_80071E20(TaskCallback callback);
void markSecondaryTaskAndClearReference(void);
