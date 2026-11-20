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

void createInputTimedSpriteTask(s32 x, s32 y, const TimedSequenceEntry *entries, s32 count,
                                s32 controllers, u32 acceptMask, u32 cancelMask, s32 depth, s32 index);

void func_80071E20(TaskCallback callback);
void markSecondaryTaskAndClearReference(void);
