#pragma once

#include "game/engine/task.h"

typedef struct SelectedSpriteResource {
    u8 unk00[0x30];
    s16 mode;
    s16 selectedIndex;
    u8 unk34[0x1C];
    u16 depth;
    u8 unk52[0xE];
    s16 sprite;
    u8 r, g, b;
} SelectedSpriteResource;
typedef struct SelectedSpriteChild {
    GameTask *task;
    s32 unk04;
} SelectedSpriteChild;

/* Six-byte descriptor selected directly or through an optional byte mapping. */
typedef struct GridSpriteEntry {
    s16 sprite;
    u8 color;
    u8 unk03[3];
} GridSpriteEntry;

typedef struct GridSpriteParent {
    u8 unk00[0x32];
    s16 selectedIndex;
    u8 unk34[0x10];
    const GridSpriteEntry *sprites;
    const u8 *mapping;
    u8 unk4C[0xE];
    u16 count;
    s8 mode;
} GridSpriteParent;

void drawOffsetSelectedTaskSprite(void);

void moveTaskChildrenWithSpacing(s32 mode, GameTask *task, s32 x, s32 y,
                                 s32 speedX, s32 speedY, s32 delay);

const GridSpriteEntry *func_800736D0(const GridSpriteEntry *sprites,
                                   const u8 *mapping, s16 index);
void drawRegisteredTaskGridSprite(void);

void drawRegisteredTaskGridSpriteWithDefaultBlend(void);
