#pragma once

#include "game/engine/task.h"
#include "game/menu/movement.h"

typedef struct SpacedGroupResource {
    Movement2D movement;
    u32 acceptMask, cancelMask;
    u8 unk30[12];
    TaskCallback callback3C, callback40;
    const u8 *data44, *data48, *data4C;
    s32 value;
    s16 mode, spacingX, spacingY, count;
    u8 unk5C, unk5D;
    s8 unk5E;
    u8 unk5F;
    s16 unk60;
    u8 r, g, b;
} SpacedGroupResource;

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
