#pragma once

#include "game/task.h"

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

void drawOffsetSelectedTaskSprite(void);

void moveTaskChildrenWithSpacing(s32 mode, GameTask *task, s32 x, s32 y,
                                 s32 speedX, s32 speedY, s32 delay);
