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
extern ByteTriplet D_8012FFFC[];
extern u8 D_8012D460[12];

void createInputTimedSpriteTask(s32 x, s32 y, const TimedSequenceEntry *entries, s32 count,
                                s32 controllers, u32 acceptMask, u32 cancelMask, s32 depth, s32 index);

s32 copyColorCycleTaskColor(s16 index, u8 *color);
s32 hasColorCycleTaskReferences(void);
void createSecondaryDisplayTask(TaskCallback callback);
void markSecondaryTaskAndClearReference(void);

void createPlayerPanelTask(s16 player, s16 x, s16 y, s32 depth, s32 index);
void createValuePanelTask(s16 x, s16 y, s16 *values, s32 depth, s32 index);
void createPaletteRotationTask(s32 x, s32 y, s32 first, s32 last, s32 steps, s32 interval, s32 index);
void configureRegisteredChildMovement(s32 index, GameTask *task, s32 x, s32 y, s32 stepX, s32 stepY, s32 delay);
void createRegisteredPanelTask(s32 mode, s32 value, s16 x, s16 y, s16 depth, s32 index);
void createRegisteredTextPanelTask(s16 x, s16 y, const u16 **texts,
                                   const u8 *mapping, s32 depth, s32 index);
void createRegisteredStaticTextPanelTask(s16 x, s16 y, s16 textX, s16 textY,
                                         const u16 *text, s32 mode, s32 depth, s32 index);
void createRegisteredDetailedTextPanelTask(s16 x, s16 y, s16 textX, s16 textY,
                                   const u16 *text, s32 value, s32 color, s32 mode,
                                   const u16 *content, s32 depth, s32 index);
void createRegisteredTripletTask(s32 x, s32 y, s32 offsetX, s32 offsetY,
                                 ByteTriplet *records, u8 *indices, s32 depth, s32 index);
void createRegisteredMovingPanelTask(u16 mode, const s16 *values, s16 x, s16 y,
                                     s16 spacingX, s16 spacingY, s16 targetX, s16 targetY,
                                     s32 depth, s32 index);
void createRegisteredTexturePairTask(s16 x, s16 y, const u8 *mapping, s32 mode,
                                     s32 unused, u16 arg5, s32 depth, s32 index);

u32 getMappedMenuWord(const u32 *entries, const u8 *remap, s16 index);
s16 mapSpacedGroupIndex(GameTask *task, s16 index);
