#include "common.h"
#include "game/save/save_data.h"
#include "game/engine/game_options.h"
#include "game/engine/byte_search.h"
#include "game/io/cd_queue.h"
#include "game/io/controller.h"
#include "game/io/controller_effects.h"
#include "game/graphics/graphics.h"
#include "game/engine/game_task_scheduler.h"
#include "game/audio/music.h"
#include "game/menu/movement.h"
#include "game/engine/task.h"
#include "game/menu/selected_sprite.h"
#include "game/menu/task_widgets.h"
#include "game/race/player/race_player.h"
#include "psx/stdlib.h"
#include "game/engine/random.h"
#include "psx/libetc.h"
#include "psx/libgpu.h"
#include "psx/libgs.h"

typedef struct SpacedTaskResource {
    s32 x, y;
    u8 unk08[0x38];
    TaskCallback callback;
    u8 unk44[0xC];
    s32 value;
    s16 unk54, spacingX, spacingY, count;
} SpacedTaskResource;

typedef struct ChildMovementGroup {
    u8 unk00[0x32];
    s16 selectedIndex;
    u8 unk34[0x22];
    s16 spacingX, spacingY, count;
} ChildMovementGroup;

typedef struct RegisteredTextResource {
    const u16 **texts;
    const u8 *mapping;
    const u16 *text;
} RegisteredTextResource;

typedef struct TexturePairResource {
    s16 x, y;
    s16 stepX, stepY;
    s8 unk08;
    u8 unk09, textureA, textureB, unk0C;
    const u8 *mapping;
} TexturePairResource;
typedef struct TexturePairParent {
    s16 unk00, selectedIndex, previousIndex, choiceResult;
} TexturePairParent;

typedef struct CallbackResource {
    TaskCallback callback;
    u32 unk04;
} CallbackResource;

typedef struct ValueTransitionResource {
    u32 *value;
    u32 target;
    u32 step;
    s16 direction;
} ValueTransitionResource;
struct GameBackground {
    u32 attribute;
    s16 x, y, w, h;
    s16 scrollX, scrollY;
    u8 r, g, b, pad;
    GameBackgroundMap *map;
    s16 mx, my, scalex, scaley;
    s32 rotate;
};
typedef struct ScrollingBackgroundResource {
    GameBackground background;
    u8 map[0x10];
    s32 x, y;
    s32 stepX, stepY;
} ScrollingBackgroundResource;

typedef struct SelectionEntry {
    s16 unk00;
    s16 selected;
    s16 previous;
    s16 unk06;
    s16 unk08;
} SelectionEntry;

typedef struct PlayerPanelAnimation {
    u8 unk00[8];
    GameTask *movingSprite;
    u8 unk0C[10];
    s16 spawnFrames;
    GameTask *stationarySprite;
} PlayerPanelAnimation;

typedef struct TimedSequence {
    const TimedSequenceEntry *entries;
    s16 index;
    s16 count;
    s32 nextFrame;
} TimedSequence;

typedef struct InputTimedSpriteResource {
    const TimedSequenceEntry *entries;
    s16 frame, count;
    s32 nextFrame;
    s32 x, y;
    u16 controllers, pad16;
    u32 acceptMask, cancelMask;
} InputTimedSpriteResource;

typedef struct SpriteZoomResource {
    union {
        s32 value;
        s16 low;
    } scale;
    u8 unk04[0x10];
    s32 step;
    u8 unk18[0xE8];
} SpriteZoomResource;

typedef struct SelectionPanelRecord {
    s16 unk00, selection, previousSelection;
} SelectionPanelRecord;
typedef struct SelectionPanelResource {
    u16 slideX;
    s16 slideY, speedX, speedY;
    s8 transitioning;
    u8 unk09;
    u8 previousTexture, nextTexture;
    u8 unk0C[8];
    SelectionPanelRecord *record;
} SelectionPanelResource;

typedef struct SelectionPanelStatsTable {
    s8 values[10][3];
    u8 padding[2];
} SelectionPanelStatsTable;

typedef struct ValuePanelResource {
    s32 x, y;
    u8 unk08[0x24];
    s16 *values;
} ValuePanelResource;

typedef struct MovingPanelState {
    s16 highlight;
    s16 selection;
    s16 respondingControllers;
    s16 closing;
} MovingPanelState;

typedef struct MovingPanelResource {
    Movement2D movement;
    MenuRgbColor color;
    u8 pad2B;
    s16 x, y;
    s16 spacingX, spacingY;
    s16 targetX, targetY;
    s16 values[6];
    u16 mode;
} MovingPanelResource;

typedef struct AnimatedSpriteResource {
    s16 frame;
    s16 unk02;
    s16 repeats;
    u16 depth;
} AnimatedSpriteResource;

typedef s16 SpriteMotionResource[128];

extern MenuRgbColor gMenuColorCycleColor __attribute__((section(".data")));
extern s16 gFrameCounter __attribute__((section(".data")));
extern u16 D_8013AF78 __attribute__((section(".data")));
extern u8 D_8013AFCD __attribute__((section(".data")));
extern u8 D_8013AFE2 __attribute__((section(".data")));
extern u8 D_8013B3A8 __attribute__((section(".data")));
extern u8 D_8013B3A9 __attribute__((section(".data")));
extern u8 D_8013B3AA __attribute__((section(".data")));
extern u8 D_8013B3AB __attribute__((section(".data")));
extern GameTask *D_8013AE88 __attribute__((section(".data")));
extern GameTask *D_8013AE8C __attribute__((section(".data")));
extern GameTask *D_8013AE98 __attribute__((section(".data")));

extern s16 D_8013B106 __attribute__((section(".data")));
extern u8 D_8013B108 __attribute__((section(".data")));

const u16 gCharacterSelectionConfirmSounds[10]
    __attribute__((section(".data.character_selection_confirm_sounds"), aligned(4))) = {
    0x0001, 0x0101, 0x0201, 0x0301, 0x0401,
    0x0501, 0x0601, 0x0701, 0x0801, 0x0901
};

const u16 gSecondaryDisplayLabel[6]
    __attribute__((section(".data.secondary_display_label"), aligned(4))) = {
    0x006D, 0x002C, 0x007F, 0x0110, 0, 0
};

TimedSequenceEntry gSecondaryTaskSequenceEntries[6]
    __attribute__((section(".data.secondary_task_sequence"))) = {
    {0x000C0172, 6}, {0x000C0173, 6}, {0x000C0174, 6},
    {0x000C0175, 6}, {0x000C0176, 6}, {0x000C0177, 6}
};

TimedSequenceEntry gCharacterPortraitSequence[4]
    __attribute__((section(".data.character_portrait_sequences"))) = {
    {0, 6}, {1, 6}, {2, 6}, {1, 6}
};

TimedSequenceEntry gExtendedCharacterPortraitSequence[8]
    __attribute__((section(".data.character_portrait_sequences"))) = {
    {0, 6}, {1, 6}, {2, 6}, {1, 6}, {2, 6}, {1, 6}, {2, 6}, {1, 6}
};

SelectionPanelStatsTable gSelectionPanelCharacterStats
    __attribute__((section(".data.selection_panel_character_stats"), aligned(4))) = {
    {
        {4, 4, 4}, {2, 6, 6}, {3, 5, 5}, {5, 3, 3}, {6, 2, 2},
        {5, 3, 3}, {4, 6, 3}, {6, 2, 3}, {5, 5, 5}, {5, 5, 5}
    },
    {0, 0}
};

u8 gSelectionPanelLabelOffsets[12]
    __attribute__((section(".data.selection_panel_label_offsets"), aligned(4))) = {
    8, 8, 16, 8, 8, 8, 0, 12, 0, 8, 0, 0
};

const u16 gValuePanelLabel[10]
    __attribute__((section(".data.value_panel_label"), aligned(4))) = {
    0x009C, 0x00BC, 0x00CE, 0x0094, 0x0094, 0x00CE,
    0x009F, 0x0098, 0x0031, 0
};

u16 gTextPanelMessage[] __attribute__((section(".data.text_panel_message"))) = {
    0xC00F, 0x0049, 0x0040, 0x0069, 0x004A, 0x0052,
    0x0158, 0x00BF, 0x0096, 0x00B9, 0x00C0, 0x00CE,
    0x009A, 0x00CB, 0x009D, 0x0094, 0x0033, 0x0000
};

extern u32 D_8013B3BC;
extern TexturedQuadListEntry *D_8013B3C0[2];
extern GameTask *D_8013B378;
extern GameTask *D_8013B37C;

s8 gTaskImageSourceRows[12] = {8, 1, 2, 3, 4, 5, 6, 7, 9, 0, 0, 0};

u8 gSaveRecordUnlockOverrideFlags
    __attribute__((section(".data.save_record_unlock_override_flags"))) = 0;

SPRT_16 gAnimatedSprite16Template __attribute__((section(".data.animated_sprite16"))) = {
    0x03000000, 0x80, 0x80, 0x80, SPRT_16_CODE,
    0, 0, 0x70, 0x50, 0x3EB2
};
s8 gSecondaryDisplayPulseGlyph[8]
    __attribute__((section(".data.secondary_display_pulse_glyph"), aligned(4))) = {
    (s8)0x81, 0x44, 0, 0, 0, 0, 0, 0
};

u8 gTexturePairBounceOffsets[8]
    __attribute__((section(".data.texture_pair_bounce_offsets"), aligned(4))) = {
    0xF8, 0, 0xFC, 0, 0xFE, 0, 0xFF, 0
};

DR_TPAGE gAnimatedSprite16DrawPage __attribute__((section(".sdata.animated_sprite16_page"))) = {
    0x01000000, {0xE1000000 | getTPage(0, 0, 832, 256)}
};

/* Interleaved signed X and Y offsets, indexed by the task's byte cursor. */
s8 gShrinkingSpriteOffsets[8] __attribute__((section(".data.shrinking_sprite_offsets"))) = {
    0, 1, 0, -1, 0, 1, 0, -1
};

void createRegisteredSpacedChildTask(s16 childIndex, s16 x, s16 y, TaskCallback callback,
                                     s32 depth, s32 registrationIndex);
void updateSpacedChildSelectionTask(void);
void updateRegisteredTexturePairTask(void);
void animateRegisteredTexturePairTask(void);

void addTilePacket(s32 x, s32 y, s32 width, s32 height, s32 r, s32 g, s32 b,
                   GsOT *ot, u16 depth);
void updateScrollingBackgroundTask(void);
void drawTaskImagesOnBlackBackground(void);
void startSecondaryDisplayTask(void);
void stopSecondaryDisplayTask(void);
void drawSecondaryDisplayTask(void);
void initTimedSequence(TimedSequence *sequence, const TimedSequenceEntry *entries, s16 index, s16 count);
void updateCountdownTask(void);
void updateRegisteredMovingPanelTask(void);
void updateInputTimedSpriteTask(void);
void updateTimedSequence(TimedSequence *sequence);
void updateTimedSpriteB5Task(void);
void updateValueTransitionTask(void);
void updateCharacterSelectionTask(void);
s32 findCharacterSelectionIndex(s16 character);
void createDualSelectionSequenceTask(s16 x, s16 y, s32 depth, s32 index);
void drawTextMessagePanel(void);
void updateCharacterSelectionPortraitTask(void);
void updatePlayerPanelTask(void);
void updatePlayerCharacterSelection(s16 playerIndex, SelectionEntry *entries);
void createRegisteredSelectionPanelTask(s16 playerIndex, SelectionPanelRecord *record, s32 depth, s32 index);
void updateRegisteredSelectionPanelTask(void);
void GsSortBg(GameBackground *background, GsOT *ot, u16 depth);
void updateShrinkingSpriteTask(void);
void updateValuePanelTask(void);
void updateAnimatedSpriteTask(void);
void updateRegisteredTextPanelTask(void);
void updateRegisteredCallbackTask(void);

void addDrawAreaPacket(s32 x, s32 y, s32 width, s32 height, GsOT *ot, u16 depth) {
    RECT area;
    DR_AREA *packet = (DR_AREA *)GsGetWorkBase();

    area.x = x;
    area.w = width;
    area.h = height;
    area.y = y + ((D_8013AF78 ^ 1) << 8);
    SetDrawArea(packet, &area);
    AddPrim(ot->org + depth, packet);
    func_800BF2C8((u32)(packet + 1));
}

void initMovement2D(Movement2D *movement, s32 x, s32 y) {
    movement->x = x;
    movement->y = y;
    movement->remainingX = 0;
    movement->remainingY = 0;
    movement->stepX = 0;
    movement->stepY = 0;
    movement->delay = 0;
}

void configureMovement2D(Movement2D *movement, s32 x, s32 y, s32 stepX, s32 stepY, s32 delay) {
    movement->remainingX = sdkAbs(x - movement->x);
    movement->remainingY = sdkAbs(y - movement->y);
    if (x - movement->x < 0) {
        movement->stepX = -sdkAbs(stepX);
    } else {
        movement->stepX = sdkAbs(stepX);
    }
    if (y - movement->y < 0) {
        movement->stepY = -sdkAbs(stepY);
    } else {
        movement->stepY = sdkAbs(stepY);
    }
    if (delay < 0) {
        movement->modeX = 2;
        movement->modeY = 2;
        if (delay == -1000) {
            delay = 0;
        } else {
            delay = sdkAbs(delay);
        }
    } else {
        movement->modeX = 0;
        movement->modeY = 0;
    }
    movement->delay = delay;
}

s32 updateMovement2D(Movement2D *movement) {
    s32 magnitude;

    if (movement->delay > 0) {
        movement->delay--;
        return 1;
    }

    if (movement->remainingX > 0) {
        magnitude = sdkAbs(movement->stepX);
        if (magnitude >= movement->remainingX) {
            if (movement->modeX == 0) {
                movement->modeX = 1;
                if (movement->stepX < 0) {
                    movement->stepX = -(movement->remainingX / 2);
                } else {
                    movement->stepX = movement->remainingX / 2;
                }
                magnitude = sdkAbs(movement->stepX);
            }
            if (magnitude >= movement->remainingX) {
                movement->x = movement->stepX < 0
                    ? movement->x - movement->remainingX
                    : movement->x + movement->remainingX;
                movement->remainingX = 0;
                goto decayX;
            }
        }
        movement->x += movement->stepX;
        movement->remainingX -= magnitude;
    decayX:
        if (movement->modeX == 1 && movement->stepX > 0x200) {
            movement->stepX /= 2;
            if (movement->stepX < 0x200) {
                movement->stepX = 0x200;
            }
        }
    }

    if (movement->remainingY > 0) {
        magnitude = sdkAbs(movement->stepY);
        if (magnitude >= movement->remainingY) {
            if (movement->modeY == 0) {
                movement->modeY = 1;
                if (movement->stepY < 0) {
                    movement->stepY = -(movement->remainingY / 2);
                } else {
                    movement->stepY = movement->remainingY / 2;
                }
                magnitude = sdkAbs(movement->stepY);
            }
            if (magnitude >= movement->remainingY) {
                movement->y = movement->stepY < 0
                    ? movement->y - movement->remainingY
                    : movement->y + movement->remainingY;
                movement->remainingY = 0;
                goto decayY;
            }
        }
        movement->y += movement->stepY;
        movement->remainingY -= magnitude;
    decayY:
        if (movement->modeY == 1 && movement->stepY > 0x200) {
            movement->stepY /= 2;
            if (movement->stepY < 0x200) {
                movement->stepY = 0x200;
            }
        }
    }

    if (movement->remainingX > 0 || movement->remainingY > 0) {
        return 1;
    }
    return 0;
}

/* DECOMP_ATTEMPT: inline 39/44 */
#ifdef NON_MATCHING
struct GameBackgroundMap {
    u8 cellw, cellh;
    u16 ncellw, ncellh;
    void *base;
    u16 *index;
};

typedef struct BackgroundTileData {
    u32 unk00;
    u8 ncellw, ncellh, cellw, cellh;
    u16 index[1];
} BackgroundTileData;

void func_80071300(s16 mode, s16 unused, GameBackground *background,
                   GameBackgroundMap *map, u8 *cells, u8 *tiles) {
    BackgroundTileData *tileData = (BackgroundTileData *)tiles;
    u32 attribute;
    attribute = mode & 3;
    cells += 8;
    map->ncellw = tileData->ncellw;
    map->ncellh = tileData->ncellh;
    map->cellw = tileData->cellw;
    tiles += 8;
    map->cellh = tileData->cellh;
    map->base = cells;
    map->index = (u16 *)tiles;
    background->r = background->g = background->b = 128;
    background->mx = 160;
    background->my = 120;
    background->scalex = background->scaley = 4096;
    background->w = 320;
    background->attribute = attribute << 24;
    background->h = 240;
    background->x = 0;
    background->y = 0;
    background->scrollX = background->scrollY = 0;
    background->map = map;
    background->rotate = 0;
    background->attribute |= 0x08000000;
}
#else
INCLUDE_ASM("asm/nonmatchings/menu/task_widgets", func_80071300);
#endif

void copyIndexedTaskImagesToVram(s16 index) {
    RECT rect;
    rect.x = 422;
    rect.y = 0;
    rect.w = 26;
    rect.h = 32;
    MoveImage(&rect, 960, 192);
    rect.x = 422;
    rect.y = 32;
    rect.w = 16;
    rect.h = 32;
    MoveImage(&rect, 986, 192);
    rect.x = 384;
    rect.y = gTaskImageSourceRows[index] * 24;
    rect.w = 38;
    rect.h = 24;
    MoveImage(&rect, 960, 168);
}

void clearTaskReference(void) {
    D_8013B378 = NULL;
}

void setTaskBackgroundMode(s32 mode, s32 value1, s32 value2) {
    ScrollingBackgroundResource *resource;
    ScrollingBackgroundResource *existing;
    switch (mode) {
    case 1:
        if (D_8013B378 != NULL) {
            if (D_8013B378->data18 == mode) return;
            D_8013B378->data18 = 0;
        }
        createChildTaskForParentWithReserve(&gGameTasks[3], drawBlackScreenTask);
        D_8013B378 = D_8013AE8C;
        D_8013B378->data18 = mode;
        break;
    case 2:
        if (D_8013B378 != NULL) {
            if (D_8013B378->data18 == mode) {
                existing = (ScrollingBackgroundResource *)&gTaskMemoryPool[D_8013B378->resource14];
                existing->stepX = value1;
                existing->stepY = value2;
                return;
            }
            D_8013B378->data18 = 0;
        }
        createChildTaskForParentWithReserve(&gGameTasks[3], updateScrollingBackgroundTask);
        D_8013AE8C->resource14 = allocateTaskMemoryBlock();
        resource = (ScrollingBackgroundResource *)&gTaskMemoryPool[D_8013AE8C->resource14];
        initDefaultBackground(0, -1, &resource->background, (GameBackgroundMap *)resource->map);
        resource->x = 0;
        resource->y = 0;
        resource->stepX = value1;
        resource->stepY = value2;
        D_8013B378 = D_8013AE8C;
        D_8013B378->data18 = 2;
        break;
    case 3:
        if (D_8013B378 != NULL) {
            if (D_8013B378->data18 == mode) return;
            D_8013B378->data18 = 0;
        }
        createChildTaskForParentWithReserve(&gGameTasks[3], drawTaskImagesOnBlackBackground);
        D_8013B378 = D_8013AE8C;
        D_8013B378->data18 = mode;
        D_8013B378->data1A = value1;
        D_8013B378->data1C = 0;
        copyIndexedTaskImagesToVram(value1);
        startSecondaryDisplayTask();
        break;
    case 0:
    default:
        if (D_8013B378 != NULL) {
            D_8013B378->data18 = 0;
            D_8013B378 = NULL;
        }
        break;
    }
}

void drawBlackScreenTask(void) {
    if (D_8013AE88->data18 == 0) {
        deleteCurrentTaskSubtree();
    } else {
        addTilePacket(-160, -120, 320, 240, 0, 0, 0,
                     &D_800EBFF0[D_8013AF78], 0xFFE);
    }
}

void updateScrollingBackgroundTask(void) {
    ScrollingBackgroundResource *resource;
    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (ScrollingBackgroundResource *)&gTaskMemoryPool[D_8013AE88->resource14];
    if (D_8013AE88->data18 == 0) {
        deleteCurrentTaskSubtree();
    } else {
        GsSortBg(&resource->background, &D_800EBFF0[D_8013AF78], 0xFFE);
        resource->x += resource->stepX;
        resource->y += resource->stepY;
        resource->background.scrollX = resource->x / 4096;
        resource->background.scrollY = resource->y / 4096;
    }
}

void drawTaskImagesOnBlackBackground(void) {
    if (D_8013AE88->data18 == 0) {
        deleteCurrentTaskSubtree();
    } else {
        setSpriteColor(128, 128, 128);
        drawSpriteByIndexWithBlend(-76, -11, 296, 0, -1, &D_800EBFF0[D_8013AF78], 1);
        drawSpriteByIndexWithBlend(-84, -16, 295, 1, -1, &D_800EBFF0[D_8013AF78], 1);
        addTilePacket(-160, -120, 320, 240, 0, 0, 0,
                      &D_800EBFF0[D_8013AF78], 0xFFE);
    }
}

s32 hasTaskReference(void) {
    return D_8013B378 != NULL;
}

void resetTaskControlState(void) {
    configureTaskStateTransition(0, 0, 30);
    setTaskBackgroundMode(0, 0, 0);
    stopSecondaryDisplayTask();
}

void startSecondaryDisplayTask(void) {
    setSecondaryDisplayTaskMode(1);
}

void stopSecondaryDisplayTask(void) {
    setSecondaryDisplayTaskMode(0);
}

void clearSecondaryTaskReference(void) {
    D_8013B37C = NULL;
}

void setSecondaryDisplayTaskMode(s16 mode) {
    switch (mode) {
    case 1: case 2: case 3:
        if (D_8013B37C != NULL) {
            D_8013B37C->data18 = 0;
        }
        createChildTaskForParentWithReserve(&gGameTasks[3], drawSecondaryDisplayTask);
        D_8013B37C = D_8013AE8C;
        D_8013AE8C->data18 = mode;
        D_8013AE8C->data1C = 0;
        D_8013AE8C->data1E = 0;
        D_8013AE8C->resource14 = allocateTaskMemoryBlock();
        initTimedSequence((TimedSequence *)&gTaskMemoryPool[D_8013AE8C->resource14], gSecondaryTaskSequenceEntries, 0, 6);
        break;
    case 0:
    default:
        if (D_8013B37C != NULL) {
            D_8013B37C->data18 = 0;
            D_8013B37C = NULL;
        }
        break;
    }
}

void drawSecondaryDisplayTask(void) {
    u8 i;
    u8 phase;
    u8 shade;

    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    if (D_8013AE88->data18 == 0) {
        deleteCurrentTaskSubtree();
    } else {
        setSpriteColor(128, 128, 128);
        drawWrappedGlyphText16x16(36, 88, 15, gSecondaryDisplayLabel, 15, -1,
                                  &D_800EBFF0[D_8013AF78], 1);
        i = 0;
        phase = D_8013AE88->data1C;
        do {
            shade = i * 32 + 16 + phase * 4;
            setSpriteColor(shade, shade, shade);
            drawWrappedDoubleByteText(i * 8 + (phase + 92), 88, 15,
                                      gSecondaryDisplayPulseGlyph, 7, 0,
                                      &D_800EBFF0[D_8013AF78], 1);
            i++;
        } while (i < 4);
        for (; i < 8; i++) {
            shade = 128 - (i - 4) * 32 - phase * 4;
            setSpriteColor(shade, shade, shade);
            drawWrappedDoubleByteText(i * 8 + (phase + 92), 88, 15,
                                      gSecondaryDisplayPulseGlyph, 7, 0,
                                      &D_800EBFF0[D_8013AF78], 1);
        }
        if (++D_8013AE88->data1C >= 8) {
            D_8013AE88->data1C = 0;
        }
        if (D_8013AE88->data18 == 1) {
            addTilePacket(-160, -120, 320, 240, 0, 0, 0,
                          &D_800EBFF0[D_8013AF78], 0xFFE);
        }
        if (D_8013AE88->data18 == 1 || D_8013AE88->data18 == 3) {
            setSpriteColor(128, 128, 128);
            if (++D_8013AE88->data1E >= 36) {
                D_8013AE88->data1E = 0;
            }
            drawSpriteByIndexWithBlend(2, 76, (u16)((u32)D_8013AE88->data1E / 6) + 370,
                          12, -1, &D_800EBFF0[D_8013AF78], 1);
        }
    }
}

void createSecondaryDisplayTask(TaskCallback callback) {
    createChildTaskForParentWithReserve(&gGameTasks[4], callback);
    D_8013B37C = D_8013AE8C;
}

void markSecondaryTaskAndClearReference(void) {
    if (D_8013B37C != NULL) {
        D_8013B37C->data18 = 99;
        D_8013B37C = NULL;
    }
}

s32 stepGrayscaleColor(TextureColor *color, s8 *stepPtr, u8 minimum, u8 maximum) {
    s32 reversed = 0;
    s8 step = *stepPtr;
    u8 value = color->r;

    if (step > 0) {
        if (value + step > maximum) {
            value = maximum;
            step = -step;
            reversed = 1;
        } else {
            value += step;
        }
    } else if (step < 0) {
        if (value - minimum < -step) {
            value = minimum;
            step = -step;
            reversed = 1;
        } else {
            value += step;
        }
    }
    color->r = color->g = color->b = value;
    *stepPtr = step;
    return reversed;
}

void drawTextureRect(s16 x, s16 y, s16 width, s16 height,
                     s16 textureDepth, s16 pageX, s16 pageY,
                     s16 clutX, s16 clutY, GsOT *ot, u16 depth) {
    POLY_FT4 *quad = &gTexturedQuadTemplate;
    s32 left, top;

    quad->clut = GetClut(clutX, clutY);
    left = x;
    top = y;
    setRGB0(quad, 128, 128, 128);
    quad->tpage = GetTPage(textureDepth, 1, pageX, pageY);
    setXY4(quad, left, top, width + left, top,
           left, height + top, width + left, height + top);
    setUV4(quad, 0, 0, width, 0, 0, height, width, height);
    GsSortPoly(quad, ot, depth);
}

void drawSemiTransparentTextureRect(s16 x, s16 y, s16 width, s16 height,
                   s16 textureDepth, s16 blendMode, s16 pageX, s16 pageY,
                   s16 clutX, s16 clutY, GsOT *ot, u16 depth) {
    POLY_FT4 *quad = &gTexturedQuadTemplate;
    s32 left, top;

    SetSemiTrans(quad, 1);
    quad->clut = GetClut(clutX, clutY);
    left = x;
    top = y;
    setRGB0(quad, 128, 128, 128);
    quad->tpage = GetTPage(textureDepth, blendMode, pageX, pageY);
    quad->x0 = left;
    quad->y0 = top;
    quad->x1 = width + left;
    quad->y1 = top;
    quad->x2 = left;
    quad->y2 = height + top;
    quad->x3 = width + left;
    quad->y3 = height + top;
    quad->u0 = 0;
    quad->v0 = 0;
    quad->u1 = width;
    quad->v1 = 0;
    quad->u2 = 0;
    quad->v2 = height;
    quad->u3 = width;
    quad->v3 = height;
    GsSortPoly(quad, ot, depth);
}

void drawSemiTransparentTextureRectUV(s16 x, s16 y, s16 width, s16 height,
                   s16 textureDepth, s16 blendMode, s16 pageX, s16 pageY,
                   u16 u, u16 v, s16 clutX, s16 clutY, GsOT *ot, u16 depth) {
    POLY_FT4 *quad = &gTexturedQuadTemplate;
    s32 left, top;

    SetSemiTrans(quad, 1);
    quad->clut = GetClut(clutX, clutY);
    left = x;
    top = y;
    setRGB0(quad, 128, 128, 128);
    quad->tpage = GetTPage(textureDepth, blendMode, pageX, pageY);
    quad->x0 = left;
    quad->y0 = top;
    quad->x1 = width + left;
    quad->y1 = top;
    quad->x2 = left;
    quad->y2 = height + top;
    quad->x3 = width + left;
    quad->y3 = height + top;
    quad->u0 = u;
    quad->v0 = v;
    quad->u1 = u + width;
    quad->v1 = v;
    quad->u2 = u;
    quad->v2 = v + height;
    quad->u3 = u + width;
    quad->v3 = v + height;
    GsSortPoly(quad, ot, depth);
}

void addTilePacket(s32 x, s32 y, s32 width, s32 height, s32 r, s32 g, s32 b,
                   GsOT *ot, u16 depth) {
    TILE *packet = (TILE *)GsGetWorkBase();
    SetTile(packet);
    setXY0(packet, x, y);
    setWH(packet, width, height);
    setRGB0(packet, r, g, b);
    AddPrim(ot->org + depth, packet);
    func_800BF2C8((u32)(packet + 1));
}

void addTilePacketWithBlend(s16 x, s16 y, s16 width, s16 height, u8 r, u8 g, u8 b,
                            s16 blendMode, GsOT *ot, u16 depth) {
    TILE *packet;
    TILE *tile;
    s32 semiTrans;
    s32 abr;
    u16 tpage;
    if (blendMode >= 0) {
        abr = blendMode;
        semiTrans = 1;
    } else {
        abr = 0;
        semiTrans = 0;
    }
    packet = (TILE *)GsGetWorkBase();
    SetTile(packet);
    tile = packet;
    setXY0(tile, x, y);
    setWH(tile, width, height);
    setRGB0(tile, r, g, b);
    SetSemiTrans(packet, semiTrans);
    AddPrim(ot->org + depth, packet);
    tpage = GetTPage(0, abr, 0, 0);
    SetDrawMode((DR_MODE *)(packet + 1), 1, 1, tpage, NULL);
    AddPrim(ot->org + depth, ++packet);
    func_800BF2C8((u32)((DR_MODE *)packet + 1));
}

void setMusicSequenceEnabled(s16 enabled) {
    if (enabled == 0) {
        if (D_8013B106 == 1) {
            fadeOutMusicSequence(0x200);
            D_8013B106 = 0;
        }
    } else if (D_8013B106 == 0) {
        playMusicSequenceAtDefaultVolume(0);
        D_8013B106 = 1;
    }
}

static inline RegisteredTaskEntry *getRegisteredTaskEntry(u32 resource, s32 index) {
    RegisteredTaskEntry *entries = (RegisteredTaskEntry *)&gTaskMemoryPool[resource];
    return &entries[index / 10];
}

void createRegisteredSpacedChildTasks(s16 controllers, u32 acceptMask, u32 cancelMask,
                                      const u8 *itemData, const u8 *mapping,
                                      const struct MenuNavigationSound *sounds, u8 arg6, s8 wrapSelection, u8 blinkFrames,
                                      s16 x, s16 y, s16 spacingX, s16 spacingY,
                                      s16 count, s16 markerSprite, TaskCallback callback3C,
                                      TaskCallback callback40, s32 value, s32 index) {
    SpacedGroupResource *state;
    RegisteredTaskEntry *entry;
    if (count != 0) {
        createChildTaskWithReserve(initializeSpacedChildTasks);
        D_8013AE8C->data1E = index;
        D_8013AE8C->resource14 = allocateTaskMemoryBlock();
        state = (SpacedGroupResource *)&gTaskMemoryPool[D_8013AE8C->resource14];
        state->controllers = controllers;
        state->spacingX = spacingX;
        state->spacingY = spacingY;
        state->count = count;
        state->unk5C = arg6;
        state->wrapSelection = wrapSelection;
        state->blinkFrames = blinkFrames;
        state->value = value;
        state->markerSprite = markerSprite;
        state->callback3C = callback3C;
        state->callback40 = callback40;
        state->itemData = itemData;
        state->mapping = mapping;
        state->sounds = sounds;
        state->acceptMask = acceptMask;
        state->cancelMask = cancelMask;
        state->color.r = 128;
        state->color.g = 128;
        state->color.b = 128;
        initMovement2D(&state->movement, x << 12, y << 12);
        D_8013AE8C->resource16 = allocateTaskMemoryBlock();
        if (index % 10 == 0) {
            if (D_8013AE88->resource14 == 0xFFFF) { while (1) {} }
            entry = getRegisteredTaskEntry(D_8013AE88->resource14, index);
        } else {
            if (D_8013AE88->resource16 == 0xFFFF) { while (1) {} }
            entry = getRegisteredTaskEntry(D_8013AE88->resource16, index);
        }
        entry->command = 0;
        entry->flags = 1;
        entry->task = D_8013AE8C;
    }
}

void initializeSpacedChildTasks(void) {
    SpacedTaskResource *state;
    RegisteredTaskEntry *entry;
    s32 i;
    RegisteredTaskEntry *entries;
    if (D_8013AE88->resource14 == 0xFFFF) { while (1) {} }
    state = (SpacedTaskResource *)&gTaskMemoryPool[D_8013AE88->resource14];
    if ((u16)(D_8013AE88->data1E % 10U) == 0) {
        if (D_8013AE98->resource14 == 0xFFFF) { while (1) {} }
        entries = (RegisteredTaskEntry *)&gTaskMemoryPool[D_8013AE98->resource14];
        entry = &entries[D_8013AE88->data1E / 10U];
    } else {
        if (D_8013AE98->resource16 == 0xFFFF) { while (1) {} }
        entries = (RegisteredTaskEntry *)&gTaskMemoryPool[D_8013AE98->resource16];
        entry = &entries[D_8013AE88->data1E / 10U];
    }
    for (i = 0; i < state->count; i++) {
        createRegisteredSpacedChildTask(i, state->x / 4096 + state->spacingX * i,
                                        state->y / 4096 + state->spacingY * i,
                                        state->callback, state->value, i * 10 + 1);
    }
    entry->flags = 0;
    setCurrentTaskCallback(updateSpacedChildSelectionTask);
}

void createRegisteredSpacedChildTask(s16 childIndex, s16 x, s16 y, TaskCallback callback,
                                     s32 depth, s32 registrationIndex) {
    ByteTriplet *color;
    RegisteredTaskEntry *entry;

    createChildTaskWithReserve(callback);
    D_8013AE8C->data18 = 0;
    D_8013AE8C->data1A = childIndex;
    D_8013AE8C->data1C = depth;
    D_8013AE8C->data1E = registrationIndex;
    D_8013AE8C->resource14 = allocateTaskMemoryBlock();
    initMovement2D((Movement2D *)&gTaskMemoryPool[D_8013AE8C->resource14], x << 12, y << 12);
    D_8013AE8C->resource16 = allocateTaskMemoryBlock();
    color = (ByteTriplet *)&gTaskMemoryPool[D_8013AE8C->resource16];
    color->bytes[0] = 128;
    color->bytes[1] = 128;
    color->bytes[2] = 128;
    if (registrationIndex % 10 == 0) {
        if (D_8013AE88->resource14 == 0xFFFF) { while (1) {} }
        entry = getRegisteredTaskEntry(D_8013AE88->resource14, registrationIndex);
    } else {
        if (D_8013AE88->resource16 == 0xFFFF) { while (1) {} }
        entry = getRegisteredTaskEntry(D_8013AE88->resource16, registrationIndex);
    }
    entry->command = 0;
    entry->flags = 0;
    entry->task = D_8013AE8C;
}

void updateSpacedChildSelectionTask(void) {
    SpacedGroupResource *group;
    FlagSelectedMenuState *parent;
    RegisteredTaskEntry *children, *slots, *slot;
    s16 i, next;
    u16 pressed = 0;
    MenuRgbColor *color;
    const MenuNavigationSound *sound;
    const GridSpriteEntry *item;

    if (D_8013AE88->resource14 == 0xFFFF) { for (;;) {} }
    group = (SpacedGroupResource *)&gTaskMemoryPool[D_8013AE88->resource14];
    if (D_8013AE88->resource16 == 0xFFFF) { for (;;) {} }
    children = (RegisteredTaskEntry *)&gTaskMemoryPool[D_8013AE88->resource16];
    if ((u16)(D_8013AE88->data1E % 10U) == 0) {
        if (D_8013AE98->resource14 == 0xFFFF) { for (;;) {} }
        slots = (RegisteredTaskEntry *)&gTaskMemoryPool[D_8013AE98->resource14];
        slot = &slots[(u16)(D_8013AE88->data1E / 10U)];
    } else {
        if (D_8013AE98->resource16 == 0xFFFF) { for (;;) {} }
        slots = (RegisteredTaskEntry *)&gTaskMemoryPool[D_8013AE98->resource16];
        slot = &slots[(u16)(D_8013AE88->data1E / 10U)];
    }
    if (D_8013AE98->resource14 == 0xFFFF) { for (;;) {} }
    parent = (FlagSelectedMenuState *)&gTaskMemoryPool[D_8013AE98->resource14];
    group->choice.mode = parent->mode;
    group->choice.selectedIndex = parent->selectedIndex;
    group->choice.previousIndex = parent->previousIndex;
    group->choice.choiceResult = parent->choiceResult;
    switch (slot->command) {
    case 1:
        for (i = 0; i < group->count; i++) {
            if (children[i].task != NULL) {
                if (i == group->choice.selectedIndex) {
                    color = (MenuRgbColor *)&gTaskMemoryPool[children[i].task->resource16];
                    color->r = 128;
                    color->g = 128;
                    color->b = 128;
                } else {
                    color = (MenuRgbColor *)&gTaskMemoryPool[children[i].task->resource16];
                    color->r = 64;
                    color->g = 64;
                    color->b = 64;
                }
            }
        }
        slot->command = 0;
        slot->flags = 0;
        break;
    case 2:
        for (i = 0; i < group->count; i++) {
            if (children[i].task != NULL) {
                color = (MenuRgbColor *)&gTaskMemoryPool[children[i].task->resource16];
                color->r = 128;
                color->g = 128;
                color->b = 128;
            }
        }
        slot->command = 0;
        slot->flags = 0;
        break;
    case 3:
        for (i = 0; i < group->count; i++) {
            if (children[i].task != NULL) {
                color = (MenuRgbColor *)&gTaskMemoryPool[children[i].task->resource16];
                color->r = 64;
                color->g = 64;
                color->b = 64;
            }
        }
        slot->command = 0;
        slot->flags = 0;
        break;
    case 10:
        if (group->blinkFrames != 0) {
            D_8013AE88->data18 = group->blinkFrames;
            slot->command = 11;
            slot->flags = 1;
        }
        break;
    case 11:
        if (D_8013AE88->data18 == 0) {
            if (children[group->choice.selectedIndex].task != NULL) {
                color = (MenuRgbColor *)&gTaskMemoryPool[children[group->choice.selectedIndex].task->resource16];
                color->r = 128;
                color->g = 128;
                color->b = 128;
            }
            slot->command = 0;
            slot->flags = 0;
        } else {
            if (D_8013AE88->data18 % 2U == 0) {
                if (children[group->choice.selectedIndex].task != NULL) {
                    color = (MenuRgbColor *)&gTaskMemoryPool[children[group->choice.selectedIndex].task->resource16];
                    color->r = 128;
                    color->g = 128;
                    color->b = 128;
                }
            } else {
                if (children[group->choice.selectedIndex].task != NULL) {
                    color = (MenuRgbColor *)&gTaskMemoryPool[children[group->choice.selectedIndex].task->resource16];
                    color->r = 64;
                    color->g = 64;
                    color->b = 64;
                }
            }
            D_8013AE88->data18--;
            slot->flags = 1;
        }
        break;
    case 20:
        group->choice.mode = 1;
        slot->command = 0;
        slot->flags = 0;
        break;
    case 21:
        group->choice.mode = 2;
        slot->command = 0;
        slot->flags = 0;
        break;
    case 22:
        group->choice.mode = 3;
        slot->command = 0;
        slot->flags = 0;
        break;
    case 23:
        group->choice.mode = 0;
        slot->command = 0;
        slot->flags = 0;
        break;
    case 0:
        slot->flags = 0;
        break;
    case 99:
    default:
        slot->task = NULL;
        deleteCurrentTaskSubtree();
        return;
    }
    for (i = 0; i < group->count; i++) {
        if (children[i].task != NULL) {
            slot->flags |= updateMovement2D((Movement2D *)&gTaskMemoryPool[children[i].task->resource14]);
        }
    }
    copyColorCycleTaskColor(0, &group->color);
    if (slot->command != 1 && slot->command != 2 && slot->command != 3 && slot->command != 11) {
        for (i = 0; i < group->count; i++) {
            if (children[i].task != NULL) {
                color = (MenuRgbColor *)&gTaskMemoryPool[children[i].task->resource16];
                if (i == group->choice.selectedIndex) {
                    color->r = 128;
                    color->g = 128;
                    color->b = 128;
                } else {
                    color->r = 64;
                    color->g = 64;
                    color->b = 64;
                }
            }
        }
    }
    if (group->choice.choiceResult == 0 && group->choice.mode > 0) {
        if (group->controllers & 1) {
            pressed |= gControllerPressedButtons[0];
        }
        if (group->controllers & 2) {
            pressed |= gControllerPressedButtons[1];
        }
        next = group->choice.selectedIndex;
        if (group->markerSprite >= 0) {
            if (pressed & PADLup) {
                if (--next < 0) {
                    if (group->wrapSelection == 1) {
                        next = group->count - 1;
                    } else {
                        next = 0;
                    }
                }
            }
            if (pressed & PADLdown) {
                if (++next >= group->count) {
                    if (group->wrapSelection == 1) {
                        next = 0;
                    } else {
                        next = group->count - 1;
                    }
                }
            }
        } else {
            if (pressed & PADLleft) {
                if (--next < 0) {
                    if (group->wrapSelection == 1) {
                        next = group->count - 1;
                    } else {
                        next = 0;
                    }
                }
            }
            if (pressed & PADLright) {
                if (++next >= group->count) {
                    if (group->wrapSelection == 1) {
                        next = 0;
                    } else {
                        next = group->count - 1;
                    }
                }
            }
            if (pressed & PADLup) {
                if (next == group->count - 1) {
                    next--;
                } else if (next >= 4) {
                    next -= 4;
                }
                if (next < 0) {
                    if (group->wrapSelection == 1) {
                        next = group->count - 1;
                    } else {
                        next = 0;
                    }
                }
            }
            if (pressed & PADLdown) {
                if (next >= 4 && next < group->count - 1) {
                    next = group->count - 1;
                } else {
                    next += 4;
                }
                if (next >= group->count) {
                    if (group->wrapSelection == 1) {
                        next = 0;
                    } else {
                        next = group->count - 1;
                    }
                }
            }
        }
        if (D_8013B108 == 3 && isControllerInputSupported(1) == 0) {
            next = 0;
        }
        if (next != group->choice.selectedIndex) {
            group->choice.previousIndex = group->choice.selectedIndex;
            group->choice.selectedIndex = next;
            if (group->sounds != NULL) {
                sound = group->sounds;
                if (sound->bank != 0xFF) {
                    playSoundEffect(sound->bank, sound->programAndTone, 32);
                }
            } else {
                playSoundEffect(0, 0x100, 32);
            }
        }
        if (pressed & group->acceptMask) {
            group->choice.choiceResult = 1;
            group->choice.mode = 3;
            item = getMappedGridSpriteEntry((const GridSpriteEntry *)group->itemData, group->mapping,
                                            group->choice.selectedIndex);
            if (item->soundBank != 0xFF) {
                playSoundEffect(item->soundBank, item->programAndTone, 32);
            }
        }
        if (pressed & group->cancelMask) {
            group->choice.choiceResult = -1;
            if (group->sounds != NULL) {
                sound = &group->sounds[1];
                if (sound->bank != 0xFF) {
                    playSoundEffect(sound->bank, sound->programAndTone, 32);
                }
            } else {
                playSoundEffect(0, 0, 32);
            }
        }
    }
    if (group->callback3C != NULL) {
        group->callback3C();
    }
    group = (SpacedGroupResource *)&gTaskMemoryPool[D_8013AE88->resource14];
    parent->mode = group->choice.mode;
    parent->selectedIndex = group->choice.selectedIndex;
    parent->previousIndex = group->choice.previousIndex;
    parent->choiceResult = group->choice.choiceResult;
}

const GridSpriteEntry *getMappedGridSpriteEntry(const GridSpriteEntry *sprites,
                                              const u8 *mapping, s16 index) {
    /* The signed right shift restores the original index on the direct path. */
    s32 selected = (s32)((u32)(u16)index << 16);
    if (mapping == NULL) {
        selected >>= 16;
    } else {
        selected = mapping[index];
    }
    return &sprites[selected];
}

u32 getMappedMenuWord(const u32 *entries, const u8 *remap, s16 index) {
    u32 offset;

    if (remap == NULL) {
        offset = index * sizeof(*entries);
    } else {
        offset = remap[index] * sizeof(*entries);
    }
    return *(const u32 *)(offset + (u32)entries);
}

s16 mapMenuByteIndex(const u8 *table, s16 index) {
    s32 value;
    if (table == NULL) {
        value = index;
    } else {
        value = table[index];
    }
    return value;
}

s16 mapSpacedGroupIndex(GameTask *task, s16 index) {
    u32 resourceIndex = task->resource14;
    const SpacedGroupResource *resource = (const SpacedGroupResource *)&gTaskMemoryPool[resourceIndex];
    const u8 *mapping = resource->mapping;
    s32 result;

    if (mapping == NULL) {
        result = index;
    } else {
        result = mapping[index];
    }
    return result;
}

s16 findByteIndexOrZero(const u8 *data, s16 value, s16 count) {
    s16 result = 0;
    s16 i;

    if (data != NULL) {
        for (i = 0; i < count; i++) {
            if (data[i] == value) {
                result = i;
                break;
            }
        }
    }
    return result;
}

void configureTaskGridMovement(s32 mode, GameTask *task, s32 x, s32 y, s32 stepX, s32 stepY, s32 delay) {
    TaskGridState *state;
    TaskGridEntry *entries;
    s16 i, end;
    state = (TaskGridState *)&gTaskMemoryPool[task->resource14];
    entries = (TaskGridEntry *)&gTaskMemoryPool[task->resource16];
    if (mode == 1) {
        i = state->selectedIndex;
        end = i + 1;
    } else {
        i = 0;
        end = state->count;
    }
    for (; i < end; i++) {
        if (mode == 2 && i == state->selectedIndex) continue;
        configureMovement2D((Movement2D *)&gTaskMemoryPool[entries[i].task->resource14], x, y, stepX, stepY, delay);
    }
}

void moveTaskChildrenWithSpacing(s32 mode, GameTask *task, s32 x, s32 y, s32 speedX, s32 speedY, s32 delay) {
    ChildMovementGroup *state;
    SelectedSpriteChild *children;
    s16 i, end;
    Movement2D *movement;
    state = (ChildMovementGroup *)&gTaskMemoryPool[task->resource14];
    children = (SelectedSpriteChild *)&gTaskMemoryPool[task->resource16];
    if (mode == 1) {
        i = state->selectedIndex;
        end = i + 1;
    } else {
        i = 0;
        end = state->count;
    }
    for (; i < end; i++) {
        if (mode == 2 && i == state->selectedIndex) {
            continue;
        }
        movement = (Movement2D *)&gTaskMemoryPool[children[i].task->resource14];
        if (delay == -1000) {
            configureMovement2D(movement, x + ((state->spacingX * i) << 12),
                           y + ((state->spacingY * i) << 12), speedX, speedY, delay);
        } else {
            configureMovement2D(movement, x + ((state->spacingX * i) << 12),
                           y + ((state->spacingY * i) << 12), speedX, speedY, delay * i);
        }
    }
}

void configureVerticalTaskGridMovement(s32 mode, GameTask *task, s32 x, s32 y, s32 stepX, s32 stepY, s32 delay) {
    TaskGridState *state;
    TaskGridEntry *entries;
    Movement2D *movement;
    s16 i, end;
    s32 nextIndex;
    state = (TaskGridState *)&gTaskMemoryPool[task->resource14];
    entries = (TaskGridEntry *)&gTaskMemoryPool[task->resource16];
    if (mode == 1) {
        i = state->selectedIndex;
        end = i + 1;
    } else {
        i = 0;
        end = state->count;
    }
    for (; i < end; i++) {
        if (mode == 2 && i == state->selectedIndex) continue;
        movement = (Movement2D *)&gTaskMemoryPool[entries[i].task->resource14];
        if (delay == -1000) {
            configureMovement2D(movement, x, y + ((state->spacingY * i) << 12), stepX, stepY, delay);
        } else {
            nextIndex = i + 1;
            configureMovement2D(movement, x, y + ((state->spacingY * i) << 12), stepX, stepY, delay * (state->count - nextIndex));
        }
    }
}

void initializeSpacedTaskGridPositions(s32 mode, GameTask *task, s32 x, s32 y, s32 spacingX, s32 spacingY) {
    TaskGridState *state;
    TaskGridEntry *entries;
    s16 i, end;
    state = (TaskGridState *)&gTaskMemoryPool[task->resource14];
    entries = (TaskGridEntry *)&gTaskMemoryPool[task->resource16];
    if (mode == 1) {
        i = state->selectedIndex;
        end = i + 1;
    } else {
        i = 0;
        end = state->count;
    }
    for (; i < end; i++) {
        if (mode == 2 && i == state->selectedIndex) continue;
        initMovement2D((Movement2D *)&gTaskMemoryPool[entries[i].task->resource14], x + spacingX * i, y + spacingY * i);
    }
}

void drawSelectedTaskSprite(void) {
    SelectedSpriteResource *state;
    SelectedSpriteChild *children;
    Movement2D *position;
    u16 mode;
    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    state = (SelectedSpriteResource *)&gTaskMemoryPool[D_8013AE88->resource14];
    if (D_8013AE88->resource16 == 0xFFFF) {
        while (1) {}
    }
    children = (SelectedSpriteChild *)&gTaskMemoryPool[D_8013AE88->resource16];
    mode = state->mode;
    if ((s16)mode > 0) {
        if ((u32)(mode - 1) < 2) {
            setSpriteColor(state->r, state->g, state->b);
        } else {
            setSpriteColor(128, 128, 128);
        }
        position = (Movement2D *)&gTaskMemoryPool[children[state->selectedIndex].task->resource14];
        drawSpriteByIndexWithBlend(position->x / 4096, position->y / 4096, state->sprite, 0, -1,
                     &D_800EBFF0[D_8013AF78], state->depth - 1);
    }
}

void drawRegisteredTaskGridSpriteWithColor(void) {
    Movement2D *position;
    ByteTriplet *color;
    GridSpriteParent *parent;
    RegisteredTaskEntry *entry;
    RegisteredTaskEntry *entries;
    const GridSpriteEntry *sprite;
    if (D_8013AE88->resource14 == 0xFFFF) { for (;;) {} }
    position = (Movement2D *)&gTaskMemoryPool[D_8013AE88->resource14];
    if (D_8013AE88->resource16 == 0xFFFF) { for (;;) {} }
    color = (ByteTriplet *)&gTaskMemoryPool[D_8013AE88->resource16];
    if (D_8013AE98->resource14 == 0xFFFF) { for (;;) {} }
    parent = (GridSpriteParent *)&gTaskMemoryPool[D_8013AE98->resource14];
    if ((u16)(D_8013AE88->data1E % 10U) == 0) {
        if (D_8013AE98->resource14 == 0xFFFF) { for (;;) {} }
        entries = (RegisteredTaskEntry *)&gTaskMemoryPool[D_8013AE98->resource14];
        entry = &entries[D_8013AE88->data1E / 10U];
    } else {
        if (D_8013AE98->resource16 == 0xFFFF) { for (;;) {} }
        entries = (RegisteredTaskEntry *)&gTaskMemoryPool[D_8013AE98->resource16];
        entry = &entries[D_8013AE88->data1E / 10U];
    }
    if (parent->mapping != 0) {
        if (entry->command == 99) {
            entry->task = 0;
            deleteCurrentTaskSubtree();
        } else {
            if (parent->mode == 1 && parent->selectedIndex != D_8013AE88->data1A) {
                sprite = getMappedGridSpriteEntry(parent->sprites, parent->mapping,
                    (s16)(D_8013AE88->data1A + parent->count));
            } else {
                sprite = getMappedGridSpriteEntry(parent->sprites, parent->mapping,
                    (s16)D_8013AE88->data1A);
            }
            setSpriteColor(color->bytes[0], color->bytes[1], color->bytes[2]);
            drawSpriteByIndexWithBlend((s16)(position->x / 4096), (s16)(position->y / 4096),
                sprite->sprite, sprite->color, -1, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
            entry->flags = 0;
        }
    }
}



void createRegisteredTextPanelTask(s16 x, s16 y, const u16 **texts, const u8 *mapping, s32 depth, s32 index) {
    RegisteredTextResource *resource;
    RegisteredTaskEntry *entry;
    createChildTaskWithReserve(updateRegisteredTextPanelTask);
    D_8013AE8C->data1A = 0;
    D_8013AE8C->data1C = depth;
    D_8013AE8C->data1E = index;
    D_8013AE8C->resource14 = allocateTaskMemoryBlock();
    initMovement2D((Movement2D *)&gTaskMemoryPool[D_8013AE8C->resource14], x << 12, y << 12);
    D_8013AE8C->resource16 = allocateTaskMemoryBlock();
    resource = (RegisteredTextResource *)&gTaskMemoryPool[D_8013AE8C->resource16];
    resource->texts = texts;
    resource->mapping = mapping;
    resource->text = 0;
    if (index % 10 == 0) {
        if (D_8013AE88->resource14 == 0xFFFF) { while (1) {} }
        entry = getRegisteredTaskEntry(D_8013AE88->resource14, index);
    } else {
        if (D_8013AE88->resource16 == 0xFFFF) { while (1) {} }
        entry = getRegisteredTaskEntry(D_8013AE88->resource16, index);
    }
    entry->command = 0;
    entry->flags = 0;
    entry->task = D_8013AE8C;
}

void updateRegisteredTextPanelTask(void) {
    Movement2D *movement;
    RegisteredTextResource *resource;
    RegisteredTaskEntry *entry;
    RegisteredTaskEntry *entries;
    SelectionEntry *selection;
    u16 mode;

    if (D_8013AE88->resource14 == 0xFFFF) { while (1) {} }
    movement = (Movement2D *)&gTaskMemoryPool[D_8013AE88->resource14];
    if (D_8013AE88->resource16 == 0xFFFF) { while (1) {} }
    resource = (RegisteredTextResource *)&gTaskMemoryPool[D_8013AE88->resource16];
    if ((u16)(D_8013AE88->data1E % 10U) == 0) {
        if (D_8013AE98->resource14 == 0xFFFF) { while (1) {} }
        entries = (RegisteredTaskEntry *)&gTaskMemoryPool[D_8013AE98->resource14];
        entry = &entries[D_8013AE88->data1E / 10U];
    } else {
        if (D_8013AE98->resource16 == 0xFFFF) { while (1) {} }
        entries = (RegisteredTaskEntry *)&gTaskMemoryPool[D_8013AE98->resource16];
        entry = &entries[D_8013AE88->data1E / 10U];
    }
    if (D_8013AE98->resource14 == 0xFFFF) { while (1) {} }
    selection = (SelectionEntry *)&gTaskMemoryPool[D_8013AE98->resource14];
    if (selection->selected != selection->previous) {
        resource->text = 0;
    }

    switch (entry->command) {
    case 20:
        D_8013AE88->data1A = 1;
        entry->command = 0;
        entry->flags = 0;
        break;
    case 21:
        D_8013AE88->data1A = 2;
        entry->command = 0;
        entry->flags = 0;
        break;
    case 22:
        D_8013AE88->data1A = 3;
        entry->command = 0;
        entry->flags = 0;
        break;
    case 23:
        D_8013AE88->data1A = 0;
        entry->command = 0;
        entry->flags = 0;
        break;
    case 0:
        entry->flags = 0;
        break;
    case 99:
    default:
        entry->task = 0;
        deleteCurrentTaskSubtree();
        return;
    }

    entry->flags |= updateMovement2D(movement);
    mode = D_8013AE88->data1A;
    if (mode != 0) {
        if ((u32)(mode - 1) < 2) {
            setSpriteColor(gMenuColorCycleColor.r, gMenuColorCycleColor.g, gMenuColorCycleColor.b);
        } else {
            setSpriteColor(128, 128, 128);
        }

        if (resource->text != 0) {
            drawWrappedGlyphText8x16((s16)(movement->x / 4096 + 4), (s16)(movement->y / 4096 + 4),
                                     14, resource->text, 7, -1, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
        } else if (resource->texts != 0) {
            s16 textIndex = mapMenuByteIndex(resource->mapping, selection->selected);

            drawWrappedGlyphText8x16((s16)(movement->x / 4096 + 4), (s16)(movement->y / 4096 + 4),
                                     14, resource->texts[textIndex], 7, -1,
                                     &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
        }
    }
    setTexturedQuadColor(128, 128, 128);
    drawDefaultTexturedPanel((s16)(movement->x / 4096), (s16)(movement->y / 4096),
                             120, 88, 5, -1, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
}

void setTaskSecondaryResourceData(GameTask *task, const u16 *data) {
    u32 index = task->resource16;
    TaskMemoryBlock *resource = &gTaskMemoryPool[index];
    resource->unk08 = data;
}

s16 hasTaskResource16Value(GameTask *task) {
    TaskMemoryBlock *resource = task->resource16 + gTaskMemoryPool;
    return resource->unk08 != 0;
}

void createRegisteredCallbackTask(s32 mode, s16 x, s16 y, TaskCallback callback, s32 value, s32 index) {
    CallbackResource *resource;
    RegisteredTaskEntry *entry;
    createChildTaskWithReserve(updateRegisteredCallbackTask);
    D_8013AE8C->data18 = mode;
    D_8013AE8C->data1A = 0;
    D_8013AE8C->data1C = value;
    D_8013AE8C->data1E = index;
    D_8013AE8C->resource14 = allocateTaskMemoryBlock();
    initMovement2D((Movement2D *)&gTaskMemoryPool[D_8013AE8C->resource14], x << 12, y << 12);
    D_8013AE8C->resource16 = allocateTaskMemoryBlock();
    resource = (CallbackResource *)&gTaskMemoryPool[D_8013AE8C->resource16];
    resource->callback = callback;
    resource->unk04 = 0;
    if (index % 10 == 0) {
        if (D_8013AE88->resource14 == 0xFFFF) { while (1) {} }
        entry = getRegisteredTaskEntry(D_8013AE88->resource14, index);
    } else {
        if (D_8013AE88->resource16 == 0xFFFF) { while (1) {} }
        entry = getRegisteredTaskEntry(D_8013AE88->resource16, index);
    }
    entry->command = 0;
    entry->flags = 0;
    entry->task = D_8013AE8C;
}

void updateRegisteredCallbackTask(void) {
    Movement2D *movement;
    CallbackResource *resource;
    RegisteredTaskEntry *entry;
    RegisteredTaskEntry *entries;
    if (D_8013AE88->resource14 == 0xFFFF) { while (1) {} }
    movement = (Movement2D *)&gTaskMemoryPool[D_8013AE88->resource14];
    if (D_8013AE88->resource16 == 0xFFFF) { while (1) {} }
    resource = (CallbackResource *)&gTaskMemoryPool[D_8013AE88->resource16];
    if ((u16)(D_8013AE88->data1E % 10U) == 0) {
        if (D_8013AE98->resource14 == 0xFFFF) { while (1) {} }
        entries = (RegisteredTaskEntry *)&gTaskMemoryPool[D_8013AE98->resource14];
        entry = &entries[D_8013AE88->data1E / 10U];
    } else {
        if (D_8013AE98->resource16 == 0xFFFF) { while (1) {} }
        entries = (RegisteredTaskEntry *)&gTaskMemoryPool[D_8013AE98->resource16];
        entry = &entries[D_8013AE88->data1E / 10U];
    }
    switch (entry->command) {
    case 20: D_8013AE88->data1A = 1; entry->command = 0; entry->flags = 0; break;
    case 21: D_8013AE88->data1A = 2; entry->command = 0; entry->flags = 0; break;
    case 22: D_8013AE88->data1A = 3; entry->command = 0; entry->flags = 0; break;
    case 23: D_8013AE88->data1A = 0; entry->command = 0; entry->flags = 0; break;
    case 0: entry->flags = 0; break;
    case 99: entry->task = 0; deleteCurrentTaskSubtree(); return;
    }
    entry->flags |= updateMovement2D(movement);
    if (D_8013AE88->data1A != 0) {
        if (D_8013AE88->data1A == 1 || D_8013AE88->data1A == 2) {
            setSpriteColor(gMenuColorCycleColor.r, gMenuColorCycleColor.g, gMenuColorCycleColor.b);
        } else {
            setSpriteColor(128, 128, 128);
        }
    }
    if (resource->callback != 0) resource->callback();
}

void createRegisteredTexturePairTask(s16 x, s16 y, const u8 *mapping, s32 mode, s32 unused, u16 arg5, s32 depth, s32 index) {
    TexturePairParent *parent;
    TexturePairResource *resource;
    RegisteredTaskEntry *entry;
    if (D_8013AE88->resource14 == 0xFFFF) { while (1) {} }
    parent = (TexturePairParent *)&gTaskMemoryPool[D_8013AE88->resource14];
    createChildTaskWithReserve(updateRegisteredTexturePairTask);
    D_8013AE8C->data1A = 0;
    D_8013AE8C->data1C = depth;
    D_8013AE8C->data1E = index;
    D_8013AE8C->resource14 = allocateTaskMemoryBlock();
    initMovement2D((Movement2D *)&gTaskMemoryPool[D_8013AE8C->resource14], x << 12, y << 12);
    D_8013AE8C->resource16 = allocateTaskMemoryBlock();
    resource = (TexturePairResource *)&gTaskMemoryPool[D_8013AE8C->resource16];
    resource->unk08 = 0;
    resource->unk0C = mode;
    resource->mapping = mapping;
    resource->textureA = mapMenuByteIndex(mapping, parent->selectedIndex);
    resource->textureB = mapMenuByteIndex(resource->mapping, parent->previousIndex);
    if (resource->textureA < 255) uploadTextureArchiveEntry(576, 256, 768, 238, resource->textureA);
    if (resource->textureB < 255) uploadTextureArchiveEntry(704, 256, 768, 237, resource->textureB);
    resource->x = 144;
    resource->y = 144;
    resource->unk08 = 0;
    resource->unk09 = arg5;
    D_8013B3BC = 0;
    if (index % 10 == 0) {
        if (D_8013AE88->resource14 == 0xFFFF) { while (1) {} }
        entry = getRegisteredTaskEntry(D_8013AE88->resource14, index);
    } else {
        if (D_8013AE88->resource16 == 0xFFFF) { while (1) {} }
        entry = getRegisteredTaskEntry(D_8013AE88->resource16, index);
    }
    entry->command = 0;
    entry->flags = 1;
    entry->task = D_8013AE8C;
}

void updateRegisteredTexturePairTask(void) {
    Movement2D *movement;
    TexturePairResource *resource;
    TexturePairParent *parent;
    RegisteredTaskEntry *entry;
    RegisteredTaskEntry *entries;
    s32 direction;
    s32 active;
    u8 color;

    if (D_8013AE88->resource14 == 0xFFFF) { while (1) {} }
    movement = (Movement2D *)&gTaskMemoryPool[D_8013AE88->resource14];
    if (D_8013AE88->resource16 == 0xFFFF) { while (1) {} }
    resource = (TexturePairResource *)&gTaskMemoryPool[D_8013AE88->resource16];
    if ((u16)(D_8013AE88->data1E % 10U) == 0) {
        if (D_8013AE98->resource14 == 0xFFFF) { while (1) {} }
        entries = (RegisteredTaskEntry *)&gTaskMemoryPool[D_8013AE98->resource14];
        entry = &entries[D_8013AE88->data1E / 10U];
    } else {
        if (D_8013AE98->resource16 == 0xFFFF) { while (1) {} }
        entries = (RegisteredTaskEntry *)&gTaskMemoryPool[D_8013AE98->resource16];
        entry = &entries[D_8013AE88->data1E / 10U];
    }
    if (D_8013AE98->resource14 == 0xFFFF) { while (1) {} }
    parent = (TexturePairParent *)&gTaskMemoryPool[D_8013AE98->resource14];
    if (entry->command != 0) {
        entry->task = 0;
        deleteCurrentTaskSubtree();
        return;
    }
    entry->flags = 0;
    entry->flags |= updateMovement2D(movement);
    setTexturedQuadColor(128, 128, 128);
    drawTexturedFrameStyle9((s16)(movement->x / 4096 - 4), (s16)(movement->y / 4096 - 4),
                            152, 152, 5, 1, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);

    if (parent->choiceResult >= 0 && resource->unk08 == 0 && parent->selectedIndex != parent->previousIndex) {
        direction = 0;
        if (parent->selectedIndex == parent->previousIndex - 1 ||
            (parent->selectedIndex == resource->unk0C - 1 && parent->previousIndex == 0)) {
            direction = 1;
        }
        if (parent->selectedIndex == parent->previousIndex + 1 ||
            (parent->selectedIndex == 0 && parent->previousIndex == resource->unk0C - 1)) {
            direction = 2;
        }
        if (resource->unk0C == 2) {
            direction = 2;
            if (parent->selectedIndex == 0) {
                direction = 1;
            }
        }
        active = 1;
        if (direction == active) {
            resource->x = 144;
            resource->y = 108;
            resource->stepX = 0;
            resource->stepY = -36;
            resource->textureA = mapMenuByteIndex(resource->mapping, parent->previousIndex);
            resource->textureB = mapMenuByteIndex(resource->mapping, parent->selectedIndex);
            resource->unk08 = active;
        }
        if (direction == 2) {
            resource->x = 144;
            resource->y = 36;
            resource->stepX = 0;
            resource->stepY = 36;
            resource->textureA = mapMenuByteIndex(resource->mapping, parent->selectedIndex);
            resource->textureB = mapMenuByteIndex(resource->mapping, parent->previousIndex);
            resource->unk08 = active;
        }
        parent->previousIndex = parent->selectedIndex;
        if (resource->textureA != resource->textureB) {
            if (resource->textureA < 255) {
                uploadTextureArchiveEntry(576, 256, 768, 238, resource->textureA);
            }
            if (resource->textureB < 255) {
                uploadTextureArchiveEntry(704, 256, 768, 237, resource->textureB);
            }
            entry->flags = active;
            setCurrentTaskCallback(animateRegisteredTexturePairTask);
        } else {
            resource->x = 144;
            resource->y = 144;
            resource->unk08 = 0;
        }
    }

    addDrawAreaPacket(0, 0, 320, 240, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
    if (resource->textureA < 255) {
        color = (D_8013B3BC & (1U << resource->textureA)) ? 40 : 128;
        setSpriteColor(color, color, color);
        drawSpriteByIndexWithBlend((s16)(movement->x / 4096 + (s16)(resource->x - 144)),
                     (s16)(movement->y / 4096 + (s16)(resource->y - 144)), 142, 0, -1,
                     &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
    }
    if (resource->textureB < 255) {
        color = (D_8013B3BC & (1U << resource->textureB)) ? 40 : 128;
        setSpriteColor(color, color, color);
        drawSpriteByIndexWithBlend((s16)(movement->x / 4096), (s16)(movement->y / 4096), 143, 0, -1,
                     &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
    }
    addDrawAreaPacket((s16)(movement->x / 4096 + 160), (s16)(movement->y / 4096 + 120),
                      144, 144, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
}

void animateRegisteredTexturePairTask(void) {
    Movement2D *movement;
    TexturePairResource *resource;
    TexturePairParent *parent;
    RegisteredTaskEntry *entry;
    RegisteredTaskEntry *entries;
    u8 color;

    if (D_8013AE88->resource14 == 0xFFFF) { while (1) {} }
    movement = (Movement2D *)&gTaskMemoryPool[D_8013AE88->resource14];
    if (D_8013AE88->resource16 == 0xFFFF) { while (1) {} }
    resource = (TexturePairResource *)&gTaskMemoryPool[D_8013AE88->resource16];
    if (D_8013AE98->resource14 == 0xFFFF) { while (1) {} }
    parent = (TexturePairParent *)&gTaskMemoryPool[D_8013AE98->resource14];
    if ((u16)(D_8013AE88->data1E % 10U) == 0) {
        if (D_8013AE98->resource14 == 0xFFFF) { while (1) {} }
        entries = (RegisteredTaskEntry *)&gTaskMemoryPool[D_8013AE98->resource14];
        entry = &entries[D_8013AE88->data1E / 10U];
    } else {
        if (D_8013AE98->resource16 == 0xFFFF) { while (1) {} }
        entries = (RegisteredTaskEntry *)&gTaskMemoryPool[D_8013AE98->resource16];
        entry = &entries[D_8013AE88->data1E / 10U];
    }
    if (entry->command != 0) {
        entry->task = 0;
        deleteCurrentTaskSubtree();
        return;
    }
    entry->flags = 0;
    entry->flags |= updateMovement2D(movement);
    if ((s8)resource->unk09 == 1 &&
        parent->selectedIndex == resource->unk0C - 1 &&
        parent->selectedIndex != parent->previousIndex) {
        resource->unk08 = 4;
    }
    if (parent->choiceResult >= 0) {
        switch (resource->unk08) {
        case 1:
            resource->x += resource->stepX;
            resource->y += resource->stepY;
            if (resource->x < 0 || resource->x > 144 ||
                resource->y < 0 || resource->y > 144) {
                D_8013AE88->data18 = 0;
                if (resource->x < 0) {
                    resource->x = 0;
                } else if (resource->x > 144) {
                    resource->x = 144;
                }
                if (resource->y < 0) {
                    resource->y = 0;
                } else if (resource->y > 144) {
                    resource->y = 144;
                }
                if ((s8)resource->unk09 == 1 && parent->selectedIndex == resource->unk0C - 1) {
                    resource->unk08 = 2;
                } else {
                    resource->unk08 = 3;
                }
            }
            break;
        case 2:
            resource->y = 144 + (s8)gTexturePairBounceOffsets[D_8013AE88->data18];
            if (++D_8013AE88->data18 >= 8) {
                resource->unk08 = 3;
            }
            break;
        case 3:
            resource->textureA = mapMenuByteIndex(resource->mapping, parent->selectedIndex);
            if (parent->selectedIndex > 0) {
                resource->textureB = mapMenuByteIndex(resource->mapping, (s16)(parent->selectedIndex - 1));
            } else {
                resource->textureB = 255;
            }
            goto upload;
        case 4:
            resource->textureA = mapMenuByteIndex(resource->mapping, parent->previousIndex);
            if (parent->selectedIndex > 0) {
                resource->textureB = mapMenuByteIndex(resource->mapping, (s16)(parent->selectedIndex - 1));
            } else {
                resource->textureB = 255;
            }
        upload:
            if (resource->textureA < 255) {
                uploadTextureArchiveEntry(576, 256, 768, 238, resource->textureA);
            }
            if (resource->textureB < 255) {
                uploadTextureArchiveEntry(704, 256, 768, 237, resource->textureB);
            }
            resource->x = 144;
            resource->y = 144;
            resource->unk08 = 0;
            entry->flags = 0;
            setCurrentTaskCallback(updateRegisteredTexturePairTask);
            break;
        }
    }
    setTexturedQuadColor(128, 128, 128);
    drawTexturedFrameStyle9((s16)(movement->x / 4096 - 4), (s16)(movement->y / 4096 - 4),
                            152, 152, 5, 1, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);

    addDrawAreaPacket(0, 0, 320, 240, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
    if (resource->textureA < 255) {
        color = (D_8013B3BC & (1U << resource->textureA)) ? 40 : 128;
        setSpriteColor(color, color, color);
        drawSpriteByIndexWithBlend((s16)(movement->x / 4096 + (s16)(resource->x - 144)),
                     (s16)(movement->y / 4096 + (s16)(resource->y - 144)), 142, 0, -1,
                     &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
    }
    if (resource->textureB < 255) {
        color = (D_8013B3BC & (1U << resource->textureB)) ? 40 : 128;
        setSpriteColor(color, color, color);
        drawSpriteByIndexWithBlend((s16)(movement->x / 4096), (s16)(movement->y / 4096), 143, 0, -1,
                     &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
    }
    addDrawAreaPacket((s16)(movement->x / 4096 + 160), (s16)(movement->y / 4096 + 120),
                      144, 144, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
}

GameTask *createValueTransitionTask(u32 *value, u32 target, u32 step) {
    ValueTransitionResource *resource;
    createChildTaskWithReserve(updateValueTransitionTask);
    D_8013AE8C->data18 = 0;
    D_8013AE8C->data1A = 0;
    D_8013AE8C->resource14 = allocateTaskMemoryBlock();
    resource = (ValueTransitionResource *)&gTaskMemoryPool[D_8013AE8C->resource14];
    resource->value = value;
    resource->target = target;
    resource->step = step;
    if (*value < target) {
        resource->direction = 1;
    } else {
        resource->direction = 2;
    }
    return D_8013AE8C;
}

void updateValueTransitionTask(void) {
    ValueTransitionResource *resource;
    u32 value;
    if (D_8013AE88->resource14 == 0xFFFF) { while (1) {} }
    resource = (ValueTransitionResource *)&gTaskMemoryPool[D_8013AE88->resource14];
    value = *resource->value;
    if (D_8013AE88->data18 != 0) {
        deleteCurrentTaskSubtree();
    } else {
        switch (resource->direction) {
        case 1:
            if (resource->step >= resource->target - value) {
                value = resource->target;
                D_8013AE88->data1A = 1;
            } else {
                value += resource->step;
                D_8013AE88->data1A = 0;
            }
            break;
        case 2:
            if (resource->step >= value - resource->target) {
                value = resource->target;
                D_8013AE88->data1A = 1;
            } else {
                value -= resource->step;
                D_8013AE88->data1A = 0;
            }
            break;
        }
        if (gControllerPressedButtons[0] != 0 || gControllerPressedButtons[1] != 0) {
            value = resource->target;
            D_8013AE88->data1A = 1;
        }
        *resource->value = value;
    }
}

GameTask *createCountdownTask(u32 frames) {
    GameTask *task;
    TaskMemoryBlock *resource;

    createChildTaskWithReserve(updateCountdownTask);
    D_8013AE8C->data18 = 0;
    D_8013AE8C->data1A = 0;
    D_8013AE8C->resource14 = allocateTaskMemoryBlock();
    task = D_8013AE8C;
    resource = &gTaskMemoryPool[task->resource14];
    resource->unk04 = frames;
    return task;
}

void updateCountdownTask(void) {
    TaskMemoryBlock *resource;

    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = &gTaskMemoryPool[D_8013AE88->resource14];
    if (D_8013AE88->data18 != 0) {
        deleteCurrentTaskSubtree();
    } else if (resource->unk04 != 0) {
        resource->unk04--;
        D_8013AE88->data1A = 0;
    } else {
        D_8013AE88->data1A = 1;
    }
}

void createRegisteredMovingPanelTask(u16 mode, const s16 *values, s16 x, s16 y,
                                     s16 spacingX, s16 spacingY, s16 targetX, s16 targetY, s32 depth, s32 index) {
    MovingPanelResource *resource;
    ValuePanelEntry *entry;
    createChildTaskWithReserve(updateRegisteredMovingPanelTask);
    D_8013AE8C->data18 = 0;
    D_8013AE8C->data1A = mode & 0xFF;
    D_8013AE8C->data1C = depth;
    D_8013AE8C->data1E = index;
    D_8013AE8C->resource14 = allocateTaskMemoryBlock();
    resource = (MovingPanelResource *)&gTaskMemoryPool[D_8013AE8C->resource14];
    resource->x = x;
    resource->y = y;
    resource->spacingX = spacingX;
    resource->spacingY = spacingY;
    resource->targetX = targetX;
    resource->targetY = targetY;
    resource->values[0] = values[0];
    resource->values[1] = values[1];
    resource->values[2] = values[2];
    resource->values[3] = values[3];
    resource->values[4] = values[4];
    resource->values[5] = values[5];
    resource->mode = (mode & 0xFFFF) >> 8;
    D_8013AE8C->resource16 = allocateTaskMemoryBlock();
    if (index % 10 == 0) {
        ValuePanelEntry *table;
        if (D_8013AE88->resource14 == 0xFFFF) { while (1) {} }
        table = (ValuePanelEntry *)&gTaskMemoryPool[D_8013AE88->resource14];
        entry = table + index / 10;
    } else {
        ValuePanelEntry *table;
        if (D_8013AE88->resource16 == 0xFFFF) { while (1) {} }
        table = (ValuePanelEntry *)&gTaskMemoryPool[D_8013AE88->resource16];
        entry = table + index / 10;
    }
    entry->flags = 1;
    entry->active = -1;
    entry->task = D_8013AE8C;
}

void updateRegisteredMovingPanelTask(void) {
    MovingPanelResource *panel;
    MovingPanelState *state;
    ValuePanelEntry *entry;
    u16 input;
    s16 selection;
    s16 x, y;
    s32 ready;
    s32 color;

    input = 0;
    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    panel = (MovingPanelResource *)&gTaskMemoryPool[D_8013AE88->resource14];
    if (D_8013AE88->resource16 == 0xFFFF) {
        while (1) {}
    }
    state = (MovingPanelState *)&gTaskMemoryPool[D_8013AE88->resource16];

    if ((u16)(D_8013AE88->data1E % 10U) == 0) {
        ValuePanelEntry *table;
        if (D_8013AE98->resource14 == 0xFFFF) {
            while (1) {}
        }
        table = (ValuePanelEntry *)&gTaskMemoryPool[D_8013AE98->resource14];
        entry = table + D_8013AE88->data1E / 10U;
    } else {
        ValuePanelEntry *table;
        if (D_8013AE98->resource16 == 0xFFFF) {
            while (1) {}
        }
        table = (ValuePanelEntry *)&gTaskMemoryPool[D_8013AE98->resource16];
        entry = table + D_8013AE88->data1E / 10U;
    }

    switch (entry->flags) {
    case 1:
        initMovement2D(&panel->movement, panel->x << 12, panel->y << 12);
        configureMovement2D(&panel->movement, panel->targetX << 12, panel->targetY << 12, 0x8000, 0x8000, 0);
        state->highlight = 0;
        state->selection = panel->mode;
        state->closing = 0;
        entry->active = -1;
        entry->flags++;
        break;
    case 2:
        if (updateMovement2D(&panel->movement) == 0) {
            configureColorCycleTask(1, 0, 128, 8, 0);
            D_8013AE88->data18 = 0;
            state->highlight = 1;
            state->respondingControllers = 0;
            entry->active = -1;
            entry->flags++;
        }
        break;
    case 3:
        ready = isTaskResourceTransitionComplete(0);
        if (ready == 1 && D_8013AE88->data18 == 0) {
            configureColorCycleTask(2, 64, 128, 4, 0);
            state->highlight = 2;
            D_8013AE88->data18 = ready;
        }
        if (D_8013AE88->data1A & 1) {
            input |= gControllerPressedButtons[0];
        }
        if (D_8013AE88->data1A & 2) {
            input |= gControllerPressedButtons[1];
        }

        selection = state->selection;
        if (input & 0x1000) {
            selection--;
        }
        if (input & 0x4000) {
            selection++;
        }
        if (selection < 0) {
            selection = 0;
        }
        if (selection >= 2) {
            selection = 1;
        }
        if (state->selection != selection) {
            state->selection = selection;
            playSoundEffect(0, 0x100, 32);
        }
        if (input & 0xB0) {
            state->closing = 1;
            playSoundEffect(0, 0, 32);
            if ((D_8013AE88->data1A & 1) && (gControllerPressedButtons[0] & 0xB0)) {
                state->respondingControllers |= 1;
            }
            if ((D_8013AE88->data1A & 2) && (gControllerPressedButtons[1] & 0xB0)) {
                state->respondingControllers |= 2;
            }
        }
        if (input & 0x40) {
            state->selection = 2;
            state->highlight = 0;
            state->closing = 1;
            playSoundEffect(0, 0, 32);
            if ((D_8013AE88->data1A & 1) && (gControllerPressedButtons[0] & 0x40)) {
                state->respondingControllers |= 1;
            }
            if ((D_8013AE88->data1A & 2) && (gControllerPressedButtons[1] & 0x40)) {
                state->respondingControllers |= 2;
            }
        }
        if (state->closing == 1) {
            configureMovement2D(&panel->movement, panel->x << 12, panel->y << 12, 0x8000, 0x8000, -1000);
            state->highlight = 0;
            entry->active = -1;
            entry->flags++;
        }
        break;
    case 4:
        if (updateMovement2D(&panel->movement) == 0) {
            entry->active = -1;
            entry->flags++;
        }
        break;
    /* Separate cleanup arms reproduce the original register allocation. */
    default:
        entry->active = state->selection;
        entry->flags = state->respondingControllers;
        deleteCurrentTaskSubtree();
        return;
    case 99:
        entry->active = state->selection;
        entry->flags = state->respondingControllers;
        deleteCurrentTaskSubtree();
        return;
    }

    x = panel->movement.x / 4096;
    y = panel->movement.y / 4096;
    if (state->highlight > 0) {
        if ((u16)state->highlight == 1 || (u16)state->highlight == 2) {
            copyColorCycleTaskColor(0, &panel->color);
            setSpriteColor(panel->color.r, panel->color.g, panel->color.b);
        } else {
            setSpriteColor(128, 128, 128);
        }
        drawSpriteByIndexWithBlend(x + state->selection * panel->spacingX, y + state->selection * panel->spacingY,
                     panel->values[4], panel->values[5], -1, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
    }

    color = state->selection == 0 ? 128 : 64;
    setSpriteColor(color, color, color);
    if (panel->targetY - 16 <= y) {
        drawSpriteByIndexWithBlend(x, y, panel->values[0], panel->values[1], -1,
                     &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
    }
    color = state->selection == 1 ? 128 : 64;
    setSpriteColor(color, color, color);
    drawSpriteByIndexWithBlend(x + panel->spacingX, y + panel->spacingY, panel->values[2], panel->values[3], -1,
                 &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
}

void createInputTimedSpriteTask(s32 x, s32 y, const TimedSequenceEntry *entries, s32 count,
    s32 controllers, u32 acceptMask, u32 cancelMask, s32 depth, s32 index) {
    InputTimedSpriteResource *resource;
    ValuePanelEntry *entry;

    createChildTaskWithReserve(updateInputTimedSpriteTask);
    D_8013AE8C->data1C = depth;
    D_8013AE8C->data1E = index;
    D_8013AE8C->resource14 = allocateTaskMemoryBlock();
    resource = (InputTimedSpriteResource *)&gTaskMemoryPool[D_8013AE8C->resource14];
    resource->entries = entries;
    resource->frame = 0;
    resource->count = count;
    resource->nextFrame = (gFrameCounter + resource->entries->duration) & 0xFFF;
    resource->x = x << 12;
    resource->y = y << 12;
    resource->controllers = controllers;
    resource->acceptMask = acceptMask;
    resource->cancelMask = cancelMask;
    if (index % 10 == 0) {
        ValuePanelEntry *table;
        if (D_8013AE88->resource14 == 0xFFFF) {
            while (1) {}
        }
        table = (ValuePanelEntry *)&gTaskMemoryPool[D_8013AE88->resource14];
        entry = table + index / 10;
    } else {
        ValuePanelEntry *table;
        if (D_8013AE88->resource16 == 0xFFFF) {
            while (1) {}
        }
        table = (ValuePanelEntry *)&gTaskMemoryPool[D_8013AE88->resource16];
        entry = table + index / 10;
    }
    entry->flags = 0;
    entry->active = 1;
    entry->task = D_8013AE8C;
}

void updateInputTimedSpriteTask(void) {
    InputTimedSpriteResource *state;
    ValuePanelEntry *entry;
    ValuePanelEntry *entries;
    u16 input = 0;
    s16 frame;
    if (D_8013AE88->resource14 == 0xFFFF) { while (1) {} }
    state = (InputTimedSpriteResource *)&gTaskMemoryPool[D_8013AE88->resource14];
    if ((u16)(D_8013AE88->data1E % 10U) == 0) {
        if (D_8013AE98->resource14 == 0xFFFF) { while (1) {} }
        entries = (ValuePanelEntry *)&gTaskMemoryPool[D_8013AE98->resource14];
        entry = &entries[D_8013AE88->data1E / 10U];
    } else {
        if (D_8013AE98->resource16 == 0xFFFF) { while (1) {} }
        entries = (ValuePanelEntry *)&gTaskMemoryPool[D_8013AE98->resource16];
        entry = &entries[D_8013AE88->data1E / 10U];
    }
    if (entry->flags == 99) {
        entry->active = 0;
        deleteCurrentTaskSubtree();
    } else {
        if (state->controllers & 1) input |= gControllerPressedButtons[0];
        if (state->controllers & 2) input |= gControllerPressedButtons[1];
        if (input & state->acceptMask) {
            playSoundEffect(0, 0, 32);
            entry->active = 0;
            deleteCurrentTaskSubtree();
        } else if (input & state->cancelMask) {
            playSoundEffect(0, 0, 32);
            entry->active = -1;
            deleteCurrentTaskSubtree();
        } else if (state->entries != NULL) {
            frame = state->frame;
            setTexturedQuadColor(128, 128, 128);
            drawTexturedQuadByIndexWithBlend((s16)(state->x / 4096), (s16)(state->y / 4096),
                (s16)state->entries[frame].unk00, (s16)(state->entries[frame].unk00 >> 16),
                -1, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
            updateTimedSequence((TimedSequence *)state);
        }
    }
}

void initTimedSequence(TimedSequence *sequence, const TimedSequenceEntry *entries, s16 index, s16 count) {
    sequence->index = index;
    sequence->entries = entries;
    sequence->count = count;
    sequence->nextFrame = (gFrameCounter + sequence->entries[index].duration) & 0xFFF;
}

void updateTimedSequence(TimedSequence *sequence) {
    if (sequence->nextFrame == gFrameCounter) {
        sequence->index++;
        if (sequence->index >= sequence->count) {
            sequence->index = 0;
        }
        sequence->nextFrame = (gFrameCounter + sequence->entries[sequence->index].duration) & 0xFFF;
    }
}

GameTask *createTimedSpriteB5Task(s16 x, s16 y, u32 frames) {
    GameTask *task;
    TaskMemoryBlock *resource;
    createChildTaskWithReserve(updateTimedSpriteB5Task);
    D_8013AE8C->data18 = 0;
    D_8013AE8C->data1A = 0;
    D_8013AE8C->resource14 = allocateTaskMemoryBlock();
    initMovement2D((Movement2D *)&gTaskMemoryPool[D_8013AE8C->resource14], x << 12, y << 12);
    D_8013AE8C->resource16 = allocateTaskMemoryBlock();
    task = D_8013AE8C;
    resource = &gTaskMemoryPool[task->resource16];
    resource->unk04 = frames;
    return task;
}

void updateTimedSpriteB5Task(void) {
    Movement2D *movement;
    TaskMemoryBlock *lifetime;
    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    movement = (Movement2D *)&gTaskMemoryPool[D_8013AE88->resource14];
    if (D_8013AE88->resource16 == 0xFFFF) {
        while (1) {}
    }
    lifetime = &gTaskMemoryPool[D_8013AE88->resource16];
    if (lifetime->unk04 == 0) {
        deleteCurrentTaskSubtree();
    }
    setSpriteColor(128, 128, 128);
    drawSpriteByIndexWithBlend(movement->x / 4096, movement->y / 4096, 0xB5, 0, -1,
                 &D_800EBFF0[D_8013AF78], 3);
    lifetime->unk04--;
}

GameTask *createAnimatedSpriteTask(s16 x, s16 y, s16 unk02, s16 repeats, u16 depth) {
    GameTask *task;
    AnimatedSpriteResource *resource;
    createChildTaskWithReserve(updateAnimatedSpriteTask);
    D_8013AE8C->data18 = 0;
    D_8013AE8C->data1A = 0;
    D_8013AE8C->resource14 = allocateTaskMemoryBlock();
    initMovement2D((Movement2D *)&gTaskMemoryPool[D_8013AE8C->resource14], x << 12, y << 12);
    D_8013AE8C->resource16 = allocateTaskMemoryBlock();
    task = D_8013AE8C;
    resource = (AnimatedSpriteResource *)&gTaskMemoryPool[task->resource16];
    resource->frame = 0;
    resource->unk02 = unk02;
    resource->depth = depth;
    resource->repeats = repeats;
    return task;
}

void updateAnimatedSpriteTask(void) {
    Movement2D *movement;
    AnimatedSpriteResource *sprite;
    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    movement = (Movement2D *)&gTaskMemoryPool[D_8013AE88->resource14];
    if (D_8013AE88->resource16 == 0xFFFF) {
        while (1) {}
    }
    sprite = (AnimatedSpriteResource *)&gTaskMemoryPool[D_8013AE88->resource16];
    if (sprite->repeats == 0) {
        deleteCurrentTaskSubtree();
    } else {
        drawAnimatedSprite16(sprite->frame, movement->x / 4096, movement->y / 4096,
                             sprite->unk02, &D_800EBFF0[D_8013AF78], sprite->depth);
        if (sprite->frame < 4) {
            sprite->frame++;
        } else {
            sprite->frame = 0;
            if (sprite->repeats > 0) {
                sprite->repeats--;
            }
        }
    }
}

void drawAnimatedSprite16(s16 frame, s16 x, s16 y, s16 unused, GsOT *ot, u16 depth) {
    SPRT_16 *packet = (SPRT_16 *)GsGetWorkBase();
    *packet = gAnimatedSprite16Template;
    packet->v0 = frame * 16 + 0x50;
    setXY0(packet, x, y);
    AddPrim(&ot->org[depth], packet);
    packet++;
    *(DR_TPAGE *)packet = gAnimatedSprite16DrawPage;
    AddPrim(&ot->org[depth], packet);
    func_800BF2C8((u32)((DR_TPAGE *)packet + 1));
}

void initializeDualResourceSequenceTask(void) {
    D_8013AE88->data18 = 1;
    D_8013AE98->data1C++;
    if (D_8013AE88->resource14 != 0xFFFF) {
        while (1) {}
    }
    D_8013AE88->resource14 = allocateTaskMemoryBlock();
    if (D_8013AE88->resource16 != 0xFFFF) {
        while (1) {}
    }
    D_8013AE88->resource16 = allocateTaskMemoryBlock();
    setCurrentTaskCallback(updateCharacterSelectionTask);
    D_8013AE88->unk10 |= 2;
}

void updateCharacterSelectionTask(void) {
    SelectionEntry *selection;
    ValuePanelEntry *panels;
    RacePlayer *players;
    s32 i;

    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    selection = (SelectionEntry *)&gTaskMemoryPool[D_8013AE88->resource14];
    if (D_8013AE88->resource16 == 0xFFFF) {
        while (1) {}
    }
    panels = (ValuePanelEntry *)&gTaskMemoryPool[D_8013AE88->resource16];

    switch (D_8013AE88->data18) {
    case 1:
        setMusicSequenceEnabled(1);
        configureTaskStateTransition(11, 0, 30);
        setSecondaryDisplayTaskMode(0);
        setTaskBackgroundMode(2, -0x1000, 0x1000);
        uploadBufferedTexturePair();
        drawTextMessagePanel();

        players = gRacePlayers;
        selection[0].unk00 = 0;
        selection[0].previous = selection[0].selected = findCharacterSelectionIndex(players[0].unk009);
        selection[0].unk06 = 0;
        if (D_8013AFCD == 2) {
            selection[1].unk00 = 0;
            selection[1].previous = selection[1].selected = findCharacterSelectionIndex(players[1].unk009);
            selection[1].unk06 = 0;
        } else {
            selection[1].unk00 = -1;
            selection[1].selected = -1;
            selection[1].previous = -1;
            selection[1].unk06 = 1;
        }

        createDualSelectionSequenceTask(-234, -100, 5, 21);
        configureMovement2D((Movement2D *)&gTaskMemoryPool[panels[2].task->resource14],
                            -144 * 4096, -100 * 4096, 15 * 4096, 0, 0);
        createPlayerPanelTask(1, -68, 184, 5, 41);
        configureMovement2D((Movement2D *)&gTaskMemoryPool[panels[4].task->resource14],
                            -68 * 4096, 4 * 4096, 0, -30 * 4096, 0);
        createPlayerPanelTask(0, -68, -280, 5, 31);
        configureMovement2D((Movement2D *)&gTaskMemoryPool[panels[3].task->resource14],
                            -68 * 4096, -100 * 4096, 0, 30 * 4096, 0);
        configureColorCycleTask(2, 64, 128, 4, 0);
        D_8013AE88->data18 = 3;
        break;

    case 3:
        drawTextMessagePanel();
        if (panels[2].active || panels[3].active || panels[4].active) {
            break;
        }
        for (i = 0; i < 2; i++) {
            if (selection[i].unk00 >= 0) {
                selection[i].unk00 = 1;
            }
        }
        D_8013B3A8 = 64;
        D_8013B3A9 = 64;
        D_8013B3AA = 64;
        D_8013B3AB = 8;
        /* Fall through. */
    case 4:
        for (i = 0; i < 2; i++) {
            if (selection[i].unk00 >= 0) {
                selection[i].unk00 = 2;
                panels[3 + i].flags = 2;
            }
        }
        D_8013AE88->data18 = 5;
        break;

    case 5:
        for (i = 0; i < 2; i++) {
            if (selection[i].unk00 > 0 && selection[i].unk06 > 0) {
                selection[i].unk00 = 3;
            }
        }
        for (i = 0; i < 2; i++) {
            if (selection[i].unk06 < 0 && selection[i].unk00 >= 0) {
                selection[i].unk06 = 0;
                selection[i].unk00 = 2;
                panels[3 + i].flags = 2;
            }
        }

        if (selection[0].unk00 >= 0 && selection[0].unk06 > 0 && (gControllerPressedButtons[0] & 0x40)) {
            selection[0].unk06 = 0;
            playSoundEffect(0, 0, 32);
        }
        if (selection[1].unk00 >= 0 && selection[1].unk06 > 0 && (gControllerPressedButtons[1] & 0x40)) {
            selection[1].unk06 = 0;
            playSoundEffect(0, 0, 32);
        }

        if (selection[0].unk06 != 0 && selection[1].unk06 != 0 &&
            panels[3].active == 0 && panels[4].active == 0) {
            createValuePanelTask(-120, -48, (s16 *)selection, 2, 51);
            D_8013AE88->data18 = 6;
        }
        break;

    case 6:
        if (panels[5].active) {
            break;
        }
        if (selection[0].unk06 != 1 || selection[1].unk06 != 1) {
            D_8013AE88->data18 = 5;
            for (i = 0; i < 2; i++) {
                if (selection[i].unk00 >= 0 && selection[i].unk06 != 1) {
                    selection[i].unk00 = 2;
                    selection[i].unk06 = 0;
                }
            }
            configureColorCycleTask(2, 64, 128, 4, 0);
        } else {
            D_8013AE98->data18 = 6;
            D_8013AFE2 = 0;
            queueMenuResourceReads((s16)D_8013AE98->data18);
            D_8013AE88->data18 = 7;
        }
        break;

    case 7:
        configureMovement2D((Movement2D *)&gTaskMemoryPool[panels[2].task->resource14],
                            -234 * 4096, -100 * 4096, -15 * 4096, 0, 0);
        configureMovement2D((Movement2D *)&gTaskMemoryPool[panels[3].task->resource14],
                            -68 * 4096, -280 * 4096, 0, -30 * 4096, 0);
        configureMovement2D((Movement2D *)&gTaskMemoryPool[panels[4].task->resource14],
                            -68 * 4096, 184 * 4096, 0, 30 * 4096, 0);
        goto advance;

    case 8:
        if (panels[2].active == 0 && panels[3].active == 0 && panels[4].active == 0) {
advance:
            D_8013AE88->data18++;
        }
        break;

    default:
        D_8013AE98->data1C--;
        deleteCurrentTaskSubtree();
        break;
    }
}

s32 findCharacterSelectionIndex(s16 character) {
    s32 result = -1;
    s32 i;

    for (i = result + 1; i < 10; i++) {
        if (character == D_8012D460[i]) {
            result = i;
            break;
        }
    }
    return result;
}

void createDualSelectionSequenceTask(s16 x, s16 y, s32 depth, s32 index) {
    SelectionEntry *selection;
    TimedSequence *sequences;
    ValuePanelEntry *entry;
    s16 i;
    u8 type;
    if (D_8013AE88->resource14 == 0xFFFF) { while (1) {} }
    selection = (SelectionEntry *)&gTaskMemoryPool[D_8013AE88->resource14];
    createChildTaskWithReserve(updateCharacterSelectionPortraitTask);
    D_8013AE8C->data1A = 0;
    D_8013AE8C->data1C = depth;
    D_8013AE8C->data1E = index;
    D_8013AE8C->resource14 = allocateTaskMemoryBlock();
    initMovement2D((Movement2D *)&gTaskMemoryPool[D_8013AE8C->resource14], x << 12, y << 12);
    D_8013AE8C->resource16 = allocateTaskMemoryBlock();
    sequences = (TimedSequence *)&gTaskMemoryPool[D_8013AE8C->resource16];
    for (i = 0; i < 2; i++) {
        type = D_8012D460[selection[i].selected];
        if (type == 3 || type == 4 || type == 8) {
            initTimedSequence(&sequences[i], gExtendedCharacterPortraitSequence, 0, 8);
        } else {
            initTimedSequence(&sequences[i], gCharacterPortraitSequence, 0, 4);
        }
    }
    if (index % 10 == 0) {
        ValuePanelEntry *table;
        if (D_8013AE88->resource14 == 0xFFFF) { while (1) {} }
        table = (ValuePanelEntry *)&gTaskMemoryPool[D_8013AE88->resource14];
        entry = table + index / 10;
    } else {
        ValuePanelEntry *table;
        if (D_8013AE88->resource16 == 0xFFFF) { while (1) {} }
        table = (ValuePanelEntry *)&gTaskMemoryPool[D_8013AE88->resource16];
        entry = table + index / 10;
    }
    entry->flags = 0;
    entry->active = 0;
    entry->task = D_8013AE8C;
}

void updateCharacterSelectionPortraitTask(void) {
    Movement2D *movement;
    TimedSequence *sequences;
    ValuePanelEntry *entry;
    SelectionEntry *selection;
    s32 sprites[2];
    s32 yOffsets[2];
    s32 depths[2];
    /* The original tests the incoming low bit of $fp when it skips the assignment below. */
    s32 shiftLastPortrait;
    s32 i;
    s32 xOffset;
    s32 frame;
    u8 type;

    if (D_8013AE88->resource14 == 0xFFFF) { while (1) {} }
    movement = (Movement2D *)&gTaskMemoryPool[D_8013AE88->resource14];
    if (D_8013AE88->resource16 == 0xFFFF) { while (1) {} }
    sequences = (TimedSequence *)&gTaskMemoryPool[D_8013AE88->resource16];

    if ((u16)(D_8013AE88->data1E % 10U) == 0) {
        ValuePanelEntry *entries;
        if (D_8013AE98->resource14 == 0xFFFF) { while (1) {} }
        entries = (ValuePanelEntry *)&gTaskMemoryPool[D_8013AE98->resource14];
        entry = &entries[D_8013AE88->data1E / 10U];
    } else {
        ValuePanelEntry *entries;
        if (D_8013AE98->resource16 == 0xFFFF) { while (1) {} }
        entries = (ValuePanelEntry *)&gTaskMemoryPool[D_8013AE98->resource16];
        entry = &entries[D_8013AE88->data1E / 10U];
    }

    if (D_8013AE98->resource14 == 0xFFFF) { while (1) {} }
    selection = (SelectionEntry *)&gTaskMemoryPool[D_8013AE98->resource14];

    if (entry->flags != 0) {
        deleteCurrentTaskSubtree();
        return;
    }

    entry->active = 0;
    entry->active |= updateMovement2D(movement);

    if (selection[0].unk00 > 0 && selection[1].unk00 > 0) {
        if (selection[0].selected != selection[1].selected) {
            sprites[0] = 42;
            sprites[1] = 43;
            yOffsets[0] = 0;
            yOffsets[1] = 0;
        } else {
            sprites[0] = 44;
            sprites[1] = 45;
            yOffsets[0] = 0;
            yOffsets[1] = 20;
        }
        if (selection[0].selected < selection[1].selected) {
            depths[0] = D_8013AE88->data1C;
            depths[1] = D_8013AE88->data1C - 1;
        } else {
            depths[0] = D_8013AE88->data1C;
            depths[1] = D_8013AE88->data1C;
        }
    } else {
        sprites[0] = 42;
        sprites[1] = 43;
        yOffsets[0] = 0;
        yOffsets[1] = 0;
        depths[0] = D_8013AE88->data1C;
        depths[1] = D_8013AE88->data1C;
    }

    if ((gMergedSaveRecord.flags & 3) == 2) {
        shiftLastPortrait = 1;
    }

    for (i = 0; i < 2; i++) {
        if (selection[i].unk00 > 0) {
            s32 selected;
            s16 x;
            if (selection[i].unk00 == 1) {
                setSpriteColor(D_8013B3A8, D_8013B3A9, D_8013B3AA);
            } else if (selection[i].unk00 == 2) {
                setSpriteColor(gMenuColorCycleColor.r, gMenuColorCycleColor.g, gMenuColorCycleColor.b);
            } else {
                setSpriteColor(128, 128, 128);
            }
            xOffset = 0;
            selected = selection[i].selected;
            if (selected == 9 && (shiftLastPortrait & 1)) {
                xOffset = -16;
            }
            x = xOffset + 4;
            x += selected % 2 * 32;
            x += movement->x / 4096;
            drawSpriteByIndexWithBlend(x, (s16)((u16)yOffsets[i] + 36 + selected / 2 * 32 + movement->y / 4096),
                                       sprites[i], (s16)i, -1, &D_800EBFF0[D_8013AF78], depths[i]);
            if (selection[i].selected != selection[i].previous) {
                type = D_8012D460[selection[i].selected];
                if (type == 3 || type == 4 || type == 8) {
                    initTimedSequence(&sequences[i], gExtendedCharacterPortraitSequence, 0, 8);
                } else {
                    initTimedSequence(&sequences[i], gCharacterPortraitSequence, 0, 4);
                }
            }
        }
    }

    setSpriteColor(128, 128, 128);
    drawSpriteByIndexWithBlend(movement->x / 4096, movement->y / 4096, 92, 13, -1,
                  &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);

    for (i = 0; i < 10; i++) {
        s16 x;
        s32 portraitBase;

        xOffset = 0;
        if (i >= 8) {
            if (!(gMergedSaveRecord.flags & (1U << (i - 8)))) {
                continue;
            }
            if (i == 9 && (shiftLastPortrait & 1)) {
                xOffset = -16;
            }
        }
        if (i == selection[0].selected || i == selection[1].selected) {
            setSpriteColor(128, 128, 128);
            if (i == selection[0].selected) {
                frame = sequences[0].index;
                frame = (s16)sequences[0].entries[frame].unk00;
            } else {
                frame = sequences[1].index;
                frame = (s16)sequences[1].entries[frame].unk00;
            }
            portraitBase = 112;
            x = xOffset + 4;
            x += i % 2 * 32;
            x += movement->x / 4096;
            drawSpriteByIndexWithBlend(x, (s16)(i / 2 * 32 + (s16)(movement->y / 4096 + 40)),
                                       D_8012D460[i] * 3 + (frame + portraitBase), 0, -1,
                                       &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
        } else {
            setSpriteColor(64, 64, 64);
            portraitBase = 112;
            x = xOffset + 4;
            x += i % 2 * 32;
            x += movement->x / 4096;
            drawSpriteByIndexWithBlend(x, (s16)(i / 2 * 32 + (s16)(movement->y / 4096 + 40)),
                                       D_8012D460[i] * 3 + portraitBase, 0, -1,
                                       &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
        }
    }

    updateTimedSequence(&sequences[0]);
    updateTimedSequence(&sequences[1]);
}

void createPlayerPanelTask(s16 player, s16 x, s16 y, s32 depth, s32 index) {
    ValuePanelEntry *entry;

    createChildTaskWithReserve(updatePlayerPanelTask);
    D_8013AE8C->data1A = player;
    D_8013AE8C->data1C = depth;
    D_8013AE8C->data1E = index;
    D_8013AE8C->resource14 = allocateTaskMemoryBlock();
    initMovement2D((Movement2D *)&gTaskMemoryPool[D_8013AE8C->resource14], x << 12, y << 12);
    D_8013AE8C->resource16 = allocateTaskMemoryBlock();
    if (index % 10 == 0) {
        ValuePanelEntry *table;
        if (D_8013AE88->resource14 == 0xFFFF) {
            while (1) {}
        }
        table = (ValuePanelEntry *)&gTaskMemoryPool[D_8013AE88->resource14];
        entry = table + index / 10;
    } else {
        ValuePanelEntry *table;
        if (D_8013AE88->resource16 == 0xFFFF) {
            while (1) {}
        }
        table = (ValuePanelEntry *)&gTaskMemoryPool[D_8013AE88->resource16];
        entry = table + index / 10;
    }
    entry->flags = 1;
    entry->active = 1;
    entry->task = D_8013AE8C;
    applySaveRecordUnlockOverrides(player);
}

void updatePlayerPanelTask(void) {
    Movement2D *position;
    PlayerPanelAnimation *animation;
    ValuePanelEntry *entries;
    ValuePanelEntry *panel;
    SelectionEntry *selections;
    SelectionEntry *selection;
    s16 player;
    s16 state;
    u8 character;
    s32 i;

    if (D_8013AE88->resource14 == 0xFFFF) { while (1) {} }
    position = (Movement2D *)&gTaskMemoryPool[D_8013AE88->resource14];
    if (D_8013AE88->resource16 == 0xFFFF) { while (1) {} }
    animation = (PlayerPanelAnimation *)&gTaskMemoryPool[D_8013AE88->resource16];
    if ((u16)(D_8013AE88->data1E % 10U) == 0) {
        if (D_8013AE98->resource14 == 0xFFFF) { while (1) {} }
        entries = (ValuePanelEntry *)&gTaskMemoryPool[D_8013AE98->resource14];
        panel = &entries[D_8013AE88->data1E / 10U];
    } else {
        if (D_8013AE98->resource16 == 0xFFFF) { while (1) {} }
        entries = (ValuePanelEntry *)&gTaskMemoryPool[D_8013AE98->resource16];
        panel = &entries[D_8013AE88->data1E / 10U];
    }
    if (D_8013AE98->resource14 == 0xFFFF) { while (1) {} }
    selections = (SelectionEntry *)&gTaskMemoryPool[D_8013AE98->resource14];
    selection = &selections[(s16)D_8013AE88->data1A];
    player = D_8013AE88->data1A;
    if (selection->selected >= 0) {
        character = D_8012D460[selection->selected];
    } else {
        character = 0;
    }

    state = panel->flags;
    if (state == 2) goto select;
    if (state < 3) {
        if (state == 0) goto clearActive;
        if (state == 1) goto initialize;
        goto remove;
    }
    if (state == 11) goto finish;
    if (state >= 12) goto remove;
    if (state == 10) goto animate;
    goto remove;

initialize:
    createRegisteredSelectionPanelTask(player, (SelectionPanelRecord *)selection,
                                       D_8013AE88->data1C + 1, 1);
    panel->flags = 0;
    panel->active = 1;
    goto render;
select:
    updatePlayerCharacterSelection(player, selections);
    if (selection->selected >= 0 && selection->unk06 > 0) {
        animation->movingSprite = createShrinkingSpriteTask(1, D_8013AE88->data1C - 2);
        animation->spawnFrames = 3;
        panel->flags = 10;
        panel->active = 1;
        goto render;
    }
    goto clearActive;
animate:
    if (animation->spawnFrames > 0) {
        animation->stationarySprite = createShrinkingSpriteTask(0, D_8013AE88->data1C - 2);
        animation->spawnFrames--;
    } else if (animation->movingSprite->data1A == 0 && animation->stationarySprite->data1A == 0) {
        panel->flags = 11;
    }
    goto render;
finish:
    if (selection->unk06 == 0) {
        selection->unk00 = 2;
        panel->flags = 2;
        animation->movingSprite->data18 = 99;
    }
clearActive:
    panel->active = 0;
    goto render;

remove:
    deleteCurrentTaskSubtree();
    return;

render:
    panel->active |= updateMovement2D(position);
    i = 0;
    if (player < D_8013AFCD) {
        for (; i < 3; i++) {
            SaveRecord *records = gPlayerSaveRecords;
            if (records[player].masks[character] & (0x1000000U << i)) {
                setSpriteColor(0x80, 0x80, 0x80);
            } else {
                setSpriteColor(0x20, 0x20, 0x20);
            }
            drawSpriteByIndexWithBlend((s16)(position->x / 4096 + 8),
                          (s16)(i * 22 + (s16)(position->y / 4096 + 30)),
                          i + 18, 6, -1, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
        }
        for (i = 0; i < 9; i++) {
            SaveRecord *records = gPlayerSaveRecords;
            u32 first;
            setSpriteColor(0x80, 0x80, 0x80);
            first = records[player].masks[character] & (1U << (i * 2));
            if (first && (records[player].masks[character] & (2U << (i * 2)))) {
                drawSpriteByIndexWithBlend((s16)(i % 3 * 16 + (s16)(position->x / 4096 + 44)),
                              (s16)(i / 3 * 22 + (s16)(position->y / 4096 + 31)),
                              i + 70, 9, -1, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
            } else if (records[player].masks[character] & (2U << (i * 2))) {
                drawSpriteByIndexWithBlend((s16)(i % 3 * 16 + (s16)(position->x / 4096 + 44)),
                              (s16)(i / 3 * 22 + (s16)(position->y / 4096 + 31)),
                              i + 70, 10, -1, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
            } else {
                u32 firstOnly = records[player].masks[character] & (1U << (i * 2));
                if (firstOnly) {
                    drawSpriteByIndexWithBlend((s16)(i % 3 * 16 + (s16)(position->x / 4096 + 44)),
                                  (s16)(i / 3 * 22 + (s16)(position->y / 4096 + 31)),
                                  i + 70, 11, -1, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
                } else {
                    setSpriteColor(0x20, 0x20, 0x20);
                    drawSpriteByIndexWithBlend((s16)(i % 3 * 16 + (s16)(position->x / 4096 + 44)),
                                  (s16)(i / 3 * 22 + (s16)(position->y / 4096 + 31)),
                                  69, 9, -1, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
                }
            }
        }
    } else {
        setSpriteColor(0x80, 0x80, 0x80);
        drawSpriteByIndexWithBlend((s16)(position->x / 4096 + 48), (s16)(position->y / 4096 + 32),
                      180, 1, -1, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
    }
    if (player < D_8013AFCD) {
        setTexturedQuadColor(0x80, 0x80, 0x80);
        setSpriteColor(0x80, 0x80, 0x80);
    } else {
        setTexturedQuadColor(0x40, 0x40, 0x40);
        setSpriteColor(0x40, 0x40, 0x40);
    }
    if (player == 0) {
        drawSpriteByIndexWithBlend((s16)(position->x / 4096 + 5), (s16)(position->y / 4096 + 2),
                      110, 6, -1, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
        drawSpriteByIndexWithBlend((s16)(position->x / 4096 + 3), (s16)(position->y / 4096 + 19),
                      91, 6, -1, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
        drawSpriteByIndexWithBlend((s16)(position->x / 4096 + 43), (s16)(position->y / 4096 + 19),
                      89, 6, -1, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
    } else {
        drawSpriteByIndexWithBlend((s16)(position->x / 4096 + 5), (s16)(position->y / 4096 + 2),
                      111, 5, -1, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
        drawSpriteByIndexWithBlend((s16)(position->x / 4096 + 3), (s16)(position->y / 4096 + 19),
                      91, 5, -1, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
        drawSpriteByIndexWithBlend((s16)(position->x / 4096 + 43), (s16)(position->y / 4096 + 19),
                      89, 5, -1, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
    }
    drawTexturedQuadList((s16)(position->x / 4096), (s16)(position->y / 4096),
                         D_8013B3C0[player], 1, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
}

void updatePlayerCharacterSelection(s16 playerIndex, SelectionEntry *entries) {
    SelectionEntry *entry = &entries[playerIndex];
    SelectionEntry *other;
    s16 previous;
    s16 selection;
    s16 upSelection;
    RacePlayer *player;
    RacePlayer *players;
    SaveRecord *records;
    const u8 *mapping;
    u8 choices[8];
    s16 count;

    if (entry->unk06 != 0 || entry->selected != entry->previous) {
        return;
    }
    if (playerIndex == 0) {
        other = &entries[1];
    } else {
        other = &entries[0];
    }
    previous = entry->selected;

    if (gControllerPressedButtons[playerIndex] & 0x8000) {
        s16 current = *(volatile s16 *)&entry->selected;
        entry->selected = (current & 1) ? current - 1 : current + 1;
        selection = entry->selected - 8;
        if ((u16)selection < 2 && !(gMergedSaveRecord.flags & (1 << selection))) {
            entry->selected = previous;
        }
    }
    if (gControllerPressedButtons[playerIndex] & 0x2000) {
        s16 current;
        entry->selected++;
        current = *(volatile s16 *)&entry->selected;
        if (!(current & 1)) {
            entry->selected = current - 2;
        }
        selection = entry->selected - 8;
        if ((u16)selection < 2 && !(gMergedSaveRecord.flags & (1 << selection))) {
            entry->selected = previous;
        }
    }
    if (gControllerPressedButtons[playerIndex] & 0x1000) {
        upSelection = entry->selected;
        entry->selected = upSelection - 2;
        if (entry->selected < 0) {
            entry->selected = upSelection + 8;
            selection = upSelection;
            if ((u16)selection < 2 && !(gMergedSaveRecord.flags & (1 << selection))) {
                entry->selected = selection + 6;
            }
        }
    }
    if (gControllerPressedButtons[playerIndex] & 0x4000) {
        selection = entry->selected;
        entry->selected = selection + 2;
        if ((u16)(selection - 6) < 2 && !(gMergedSaveRecord.flags & (1 << (s16)(selection - 6)))) {
            entry->selected = selection + 4;
        }
        if (entry->selected >= 10) {
            entry->selected -= 10;
        }
    }
    if (entry->selected != previous) {
        entry->previous = previous;
        playSoundEffect(0, 0x100, 0x20);
    }
    if (gControllerPressedButtons[playerIndex] & 0xB0) {
        entry->unk06 = 1;
        mapping = &D_8012D460[entry->selected];
        records = gPlayerSaveRecords;
        players = gRacePlayers;
        player = &players[playerIndex];
        player->unk009 = *mapping;
        player->unk00A[0] = records[playerIndex].boardSelections[player->unk009];
        player->unk00D = records[playerIndex].paletteSelections[player->unk009];
        if (D_8013AFCD != 2 || other->unk00 < 0 || other->unk06 != 1 ||
            gRacePlayers[0].unk009 != gRacePlayers[1].unk009) {
            goto confirm;
        }
        if (gRacePlayers[0].unk00A[0] == gRacePlayers[1].unk00A[0]) {
            count = 0;
            for (selection = 0; selection < 4; selection++) {
                if (selection != gRacePlayers[playerIndex].unk00A[0]) {
                    choices[count] = selection;
                    count++;
                }
            }
            gRacePlayers[playerIndex].unk00A[0] = choices[rand() % count];
        }
        count = 0;
        if (gRacePlayers[0].unk00D == gRacePlayers[1].unk00D) {
            for (selection = 0; selection < 8; selection++) {
                if (selection != gRacePlayers[playerIndex].unk00D) {
                    choices[count] = selection;
                    count++;
                }
            }
            gRacePlayers[playerIndex].unk00D = choices[rand() % count];
        }
confirm:
        playSoundEffect(1, gCharacterSelectionConfirmSounds[D_8012D460[entry->selected]], 0x20);
    }
    if (gControllerPressedButtons[playerIndex] & 0x40) {
        entry->unk06 = -1;
    }
}

void createRegisteredSelectionPanelTask(s16 playerIndex, SelectionPanelRecord *record, s32 depth, s32 index) {
    SelectionPanelResource *resource;
    ValuePanelEntry *entry;
    s32 player;
    createChildTaskWithReserve(updateRegisteredSelectionPanelTask);
    D_8013AE8C->data1A = playerIndex;
    D_8013AE8C->data1C = depth;
    D_8013AE8C->data1E = index;
    D_8013AE8C->resource14 = allocateTaskMemoryBlock();
    resource = (SelectionPanelResource *)&gTaskMemoryPool[D_8013AE8C->resource14];
    resource->transitioning = 0;
    resource->record = record;
    player = playerIndex;
    if (player < D_8013AFCD) {
        uploadTextureArchiveEntry(0x240 + (player << 7), 0x100, 0x300,
                                  0xEE - player * 2, D_8012D460[record->selection]);
    } else {
        uploadTextureArchiveEntry(0x240 + (player << 7), 0x100, 0x300,
                                  0xEE - player * 2, 10);
    }
    uploadTextureArchiveEntry(0x140, 0x100, 0x300, 0xEA, 11);
    if (index % 10 == 0) {
        ValuePanelEntry *table;
        if (D_8013AE88->resource14 == 0xFFFF) { while (1) {} }
        table = (ValuePanelEntry *)&gTaskMemoryPool[D_8013AE88->resource14];
        entry = table + index / 10;
    } else {
        ValuePanelEntry *table;
        if (D_8013AE88->resource16 == 0xFFFF) { while (1) {} }
        table = (ValuePanelEntry *)&gTaskMemoryPool[D_8013AE88->resource16];
        entry = table + index / 10;
    }
    entry->flags = 0;
    entry->active = 0;
    entry->task = D_8013AE8C;
}

void updateRegisteredSelectionPanelTask(void) {
    SelectionPanelResource *resource;
    Movement2D *position;
    SelectionPanelRecord *record;
    TaskMemoryBlock *slot;
    GameTask *parent;
    s16 player;
    u8 shade;

    if (D_8013AE88->resource14 == 0xFFFF) { while (1) {} }
    slot = &gTaskMemoryPool[D_8013AE88->resource14];
    if (D_8013AE98->resource14 == 0xFFFF) { while (1) {} }
    parent = D_8013AE98;
    position = (Movement2D *)&gTaskMemoryPool[parent->resource14];
    if ((u16)(D_8013AE88->data1E % 10U) == 0) {
        if (parent->resource14 == 0xFFFF) { while (1) {} }
    } else {
        if (parent->resource16 == 0xFFFF) { while (1) {} }
    }

    resource = (SelectionPanelResource *)slot;
    player = D_8013AE88->data1A;
    record = resource->record;
    shade = 128;
    setSpriteColor(shade, shade, shade);
    if (player >= D_8013AFCD) {
        setSpriteColor(64, 64, 64);
        drawSpriteByIndexWithBlend((s16)(position->x / 4096 + 96), (s16)(position->y / 4096),
                     241 + player * 2, 0, -1, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
    } else {
        if (!resource->transitioning && record->selection != record->previousSelection) {
            resource->transitioning = 1;
            resource->slideX = 112;
            resource->slideY = 0;
            resource->speedX = -28;
            resource->speedY = 0;
            resource->previousTexture = D_8012D460[record->previousSelection];
            resource->nextTexture = D_8012D460[record->selection];
            uploadTextureArchiveEntry(576 + player * 128, 256, 768, 238 - player * 2,
                                      resource->previousTexture);
            uploadTextureArchiveEntry(640 + player * 128, 256, 768, 237 - player * 2,
                                      resource->nextTexture);
        }
        if (resource->transitioning == 1) {
            resource->slideX += resource->speedX;
            resource->slideY += resource->speedY;
            if (resource->slideX > 112 || resource->slideY < 0 || resource->slideY > 112) {
                resource->transitioning = 0;
                record->previousSelection = record->selection;
                uploadTextureArchiveEntry(576 + player * 128, 256, 768, 238 - player * 2,
                                          D_8012D460[record->selection]);
            }
        }

        drawSpriteByIndexWithBlend((s16)(position->x / 4096 + 112), (s16)(position->y / 4096 + 70),
                     30, 1, -1, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
        drawSpriteByIndexWithBlend((s16)(position->x / 4096 + 112), (s16)(position->y / 4096 + 78),
                     31, 1, -1, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
        drawSpriteByIndexWithBlend((s16)(position->x / 4096 + 112), (s16)(position->y / 4096 + 86),
                     32, 1, -1, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
        if (resource->transitioning == 1) {
            addDrawAreaPacket(0, 0, 320, 240, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
            {
                s32 slideX = position->x / 4096 - 16;
                drawSpriteByIndexWithBlend((s16)(slideX + (s32)resource->slideX),
                             (s16)(position->y / 4096), 241 + player * 2, 0, 0,
                             &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
            }
            drawSpriteByIndexWithBlend((s16)(resource->slideX + 96 + position->x / 4096),
                         (s16)(position->y / 4096), 242 + player * 2, 0, 0,
                         &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
            drawSpriteByIndexWithBlend((s16)(position->x / 4096 + 96), (s16)(position->y / 4096),
                         267, 0, -1, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
            addDrawAreaPacket((s16)(position->x / 4096 + 256), (s16)(position->y / 4096 + 120),
                              112, 96, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
        } else {
            s16 character = D_8012D460[record->selection];
            const s8 *stats = gSelectionPanelCharacterStats.values[character];
            s8 labelOffset;

            drawHalfStepSpriteBar((s16)(position->x / 4096 + 168), (s16)(position->y / 4096 + 70),
                                  stats[0], 5, -1, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
            drawHalfStepSpriteBar((s16)(position->x / 4096 + 168), (s16)(position->y / 4096 + 78),
                                  stats[1], 5, -1, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
            drawHalfStepSpriteBar((s16)(position->x / 4096 + 168), (s16)(position->y / 4096 + 86),
                                  stats[2], 5, -1, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
            drawSpriteByIndexWithBlend((s16)(position->x / 4096 + 96), (s16)(position->y / 4096),
                         241 + player * 2, 0, 0, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
            drawSpriteByIndexWithBlend((s16)(position->x / 4096 + 96), (s16)(position->y / 4096),
                         267, 0, -1, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
            labelOffset = gSelectionPanelLabelOffsets[character];
            drawSpriteByIndexWithBlend((s16)(labelOffset + (s16)(position->x / 4096 + 33)),
                         (s16)(position->y / 4096 + 3), 254 + character,
                         (s16)(player + 2), -1,
                         &D_800EBFF0[D_8013AF78], (u16)(D_8013AE88->data1C - 2));
        }
    }
}

void createValuePanelTask(s16 x, s16 y, s16 *values, s32 depth, s32 index) {
    ValuePanelResource *resource;
    ValuePanelEntry *entry;

    createChildTaskWithReserve(updateValuePanelTask);
    D_8013AE8C->data1C = depth;
    D_8013AE8C->data1E = index;
    D_8013AE8C->resource14 = allocateTaskMemoryBlock();
    resource = (ValuePanelResource *)&gTaskMemoryPool[D_8013AE8C->resource14];
    resource->values = values;
    resource->x = x << 12;
    resource->y = y << 12;
    D_8013AE8C->resource16 = allocateTaskMemoryBlock();
    if (index % 10 == 0) {
        ValuePanelEntry *table;
        if (D_8013AE88->resource14 == 0xFFFF) {
            while (1) {}
        }
        table = (ValuePanelEntry *)&gTaskMemoryPool[D_8013AE88->resource14];
        entry = table + index / 10;
    } else {
        ValuePanelEntry *table;
        if (D_8013AE88->resource16 == 0xFFFF) {
            while (1) {}
        }
        table = (ValuePanelEntry *)&gTaskMemoryPool[D_8013AE88->resource16];
        entry = table + index / 10;
    }
    entry->flags = 1;
    entry->active = 1;
    entry->task = D_8013AE8C;
}

void updateValuePanelTask(void) {
    ValuePanelResource *resource;
    ValuePanelEntry *result;
    ValuePanelEntry *entry;
    ValuePanelEntry *entries;
    s16 *values;
    s16 panelValues[6];
    u16 mode;

    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (ValuePanelResource *)&gTaskMemoryPool[D_8013AE88->resource14];
    if (D_8013AE88->resource16 == 0xFFFF) {
        while (1) {}
    }
    result = (ValuePanelEntry *)&gTaskMemoryPool[D_8013AE88->resource16];
    if ((u16)(D_8013AE88->data1E % 10U) == 0) {
        if (D_8013AE98->resource14 == 0xFFFF) {
            while (1) {}
        }
        entries = (ValuePanelEntry *)&gTaskMemoryPool[D_8013AE98->resource14];
        entry = &entries[D_8013AE88->data1E / 10U];
    } else {
        if (D_8013AE98->resource16 == 0xFFFF) {
            while (1) {}
        }
        entries = (ValuePanelEntry *)&gTaskMemoryPool[D_8013AE98->resource16];
        entry = &entries[D_8013AE88->data1E / 10U];
    }

    values = resource->values;
    switch (entry->flags) {
    case 1:
        /* The inverted sign bit enables the first option. */
        mode = (u32)~(s32)values[1] >> 31;
        if (values[6] >= 0) {
            mode |= 2;
        }
        panelValues[0] = 21;
        panelValues[1] = 5;
        panelValues[2] = 22;
        panelValues[3] = 5;
        panelValues[4] = 23;
        panelValues[5] = 5;
        createRegisteredMovingPanelTask(mode, panelValues, -40, -16, 0, 16,
                                        -40, 16, D_8013AE88->data1C - 1, 1);
        entry->flags = 2;
        entry->active = 1;
        break;
    case 2:
        if (result->active >= 0) {
            if (result->active != 0) {
                if (result->flags & 1) {
                    values[3] = 0;
                }
                if (result->flags & 2) {
                    values[8] = 0;
                }
            }
            entry->flags = 99;
        }
        break;
    case 99:
    default:
        entry->active = 0;
        deleteCurrentTaskSubtree();
        return;
    }
    setSpriteColor(128, 128, 128);
    setTexturedQuadColor(128, 128, 128);
    drawWrappedGlyphText16x16((s16)(resource->x / 4096 + 48), (s16)(resource->y / 4096 + 16),
                              14, gValuePanelLabel, 15, -1, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C - 2);
    drawDefaultTexturedPanel((s16)(resource->x / 4096), (s16)(resource->y / 4096),
                             240, 64, 5, -1, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C - 2);
    drawScreenDarkeningOverlay(64, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
}

GameTask *createShrinkingSpriteTask(s16 motionEnabled, s32 spriteIndex) {
    SpriteZoomResource *resource;
    TaskMemoryBlock *resources;
    createChildTaskWithReserve(updateShrinkingSpriteTask);
    D_8013AE8C->data18 = 1;
    D_8013AE8C->data1A = 1;
    D_8013AE8C->data1C = spriteIndex;
    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resources = gTaskMemoryPool;
    D_8013AE8C->resource14 = allocateTaskMemoryBlock();
    resource = (SpriteZoomResource *)&resources[D_8013AE8C->resource14];
    resource->scale.value = 0x8000;
    resource->step = 0x400;
    D_8013AE8C->resource16 = allocateTaskMemoryBlock();
    (*(SpriteMotionResource *)&resources[D_8013AE8C->resource16])[0] = motionEnabled;
    return D_8013AE8C;
}

void updateShrinkingSpriteTask(void) {
    SpriteZoomResource *zoom;
    SpriteMotionResource *motion;
    Movement2D *position;
    GameTask *task;
    u16 depth;

    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    zoom = (SpriteZoomResource *)&gTaskMemoryPool[D_8013AE88->resource14];
    if (D_8013AE88->resource16 == 0xFFFF) {
        while (1) {}
    }
    motion = (SpriteMotionResource *)&gTaskMemoryPool[D_8013AE88->resource16];
    if (D_8013AE98->resource14 == 0xFFFF) {
        while (1) {}
    }
    position = (Movement2D *)&gTaskMemoryPool[D_8013AE98->resource14];
    task = D_8013AE88;
    switch (task->data18) {
    case 1:
        if (zoom->scale.value > 0x1000) {
            zoom->scale.value -= zoom->step;
            zoom->step *= 2;
            if (zoom->scale.value < 0x1000) {
                zoom->scale.value = 0x1000;
            }
        } else {
            task->data1E = 0;
            task->data18 = 2;
        }
        break;
    case 2:
        if ((*motion)[0]) {
            position->x += gShrinkingSpriteOffsets[task->data1E] << 12;
            position->y += (gShrinkingSpriteOffsets + 1)[task->data1E] << 12;
        }
        D_8013AE88->data1E += 2;
        if (D_8013AE88->data1E >= 8) {
            D_8013AE88->data1A = 0;
            if (!(*motion)[0]) {
                D_8013AE88->data18 = 99;
            } else {
                D_8013AE88->data18 = 3;
            }
        }
        break;
    case 0:
    case 99:
        deleteCurrentTaskSubtree();
        break;
    }

    setSpriteColor(128, 128, 128);
    if (!(*motion)[0]) {
        depth = D_8013AE88->data1C - 1;
        drawTransformedSpriteByIndex((s16)(position->x / 4096 + 155),
                                     (s16)(position->y / 4096 + 49), 88, 12,
                                     3, 0x7FFF, 0x7FFF, zoom->scale.low,
                                     zoom->scale.low, 0, &D_800EBFF0[D_8013AF78], depth);
    } else {
        if (zoom->scale.value > 0x1000) {
            depth = D_8013AE88->data1C - 1;
        } else {
            depth = D_8013AE88->data1C;
        }
        drawTransformedSpriteByIndex((s16)(position->x / 4096 + 155),
                                     (s16)(position->y / 4096 + 49), 88, 12,
                                     -1, 0x7FFF, 0x7FFF, zoom->scale.low,
                                     zoom->scale.low, 0, &D_800EBFF0[D_8013AF78], depth);
    }
}

/* DECOMP_ATTEMPT: inline 235/257 */
#ifdef NON_MATCHING
void cycleSaveRecordUnlockFlags(s16 recordIndex, s16 maskIndex);

void func_8007B48C(s16 recordIndex, s16 maskIndex) {
    s16 row;
    s16 paired[2];
    SaveRecord *state;
    SaveRecord *records;

    if (gControllerPressedButtons[recordIndex] & 8) {
        cycleSaveRecordUnlockFlags(recordIndex, maskIndex);
    } else if (gControllerPressedButtons[recordIndex] & 2) {
        if (getSaveRecordMaskTier(recordIndex, maskIndex)) {
            for (maskIndex = 0; maskIndex < 10; maskIndex++) {
                setSaveRecordMaskTier(recordIndex, maskIndex, 4);
                cycleSaveRecordUnlockFlags(recordIndex, maskIndex);
            }
        } else {
            for (maskIndex = 0; maskIndex < 10; maskIndex++) {
                setSaveRecordMaskTier(recordIndex, maskIndex, 4);
                cycleSaveRecordUnlockFlags(recordIndex, maskIndex);
                cycleSaveRecordUnlockFlags(recordIndex, maskIndex);
                cycleSaveRecordUnlockFlags(recordIndex, maskIndex);
                cycleSaveRecordUnlockFlags(recordIndex, maskIndex);
                cycleSaveRecordUnlockFlags(recordIndex, maskIndex);
                cycleSaveRecordUnlockFlags(recordIndex, maskIndex);
            }
        }

        records = gPlayerSaveRecords;
        state = &records[recordIndex];
        /* Diagnose the original read of the row register before initialization. */
        state->masks[maskIndex] |= 0x1000000U << row;
        state->flags = 0x3FFF;
        for (row = 0; row < 5; row++) {
            state->boardUnlockFlagsLow |= 1U << (row * 5 + 1);
            state->boardUnlockFlagsHigh |= 1U << (row * 5 + 1);
            state->boardUnlockFlagsLow |= 1U << (row * 5 + 2);
            state->boardUnlockFlagsHigh |= 1U << (row * 5 + 2);
            state->boardUnlockFlagsLow |= 1U << (row * 5 + 3);
            state->boardUnlockFlagsLow |= 1U << (row * 5 + 4);
            state->boardUnlockFlagsHigh |= 1U << (row * 5 + 3);
            state->boardUnlockFlagsHigh |= 1U << (row * 5 + 4);
        }
        gPlayerSaveRecords[recordIndex].money = 999999;
        rebuildCombinedStateMask(&gPlayerSaveRecords[recordIndex]);
        copySaveRecordAndMergeFlags();
        for (row = 0; row < 11; row++) {
            setUnlockNotificationFlag(row);
        }
    }

    if (gControllerPressedButtons[recordIndex] & 4) {
        s32 flag;
        paired[0] = getGlobalFlagMask(0);
        paired[1] = getGlobalFlagMask(1);
        if (paired[0]) {
            if (!paired[1]) {
                goto enableSecond;
            }
            flag = -1;
        } else {
            flag = 0;
            if (!paired[1]) {
enableSecond:
                flag = 1;
            }
        }
        updatePairedStateFlag(flag);
    }
    if (gControllerPressedButtons[recordIndex] & 1) {
        if (getSaveRecordFlagPair(recordIndex, maskIndex, 0) > 0) {
            setSaveRecordFlagPair(recordIndex, maskIndex, 0, 0);
            setSaveRecordFlagPair(recordIndex, maskIndex, 1, 0);
            setSaveRecordFlagPair(recordIndex, maskIndex, 2, 0);
        } else {
            setSaveRecordFlagPair(recordIndex, maskIndex, 0, 3);
            setSaveRecordFlagPair(recordIndex, maskIndex, 1, 2);
            setSaveRecordFlagPair(recordIndex, maskIndex, 2, 1);
        }
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/menu/task_widgets", func_8007B48C);
#endif

void cycleSaveRecordUnlockFlags(s16 recordIndex, s16 maskIndex) {
    s16 pairIndex;
    s16 flags;

    switch (getSaveRecordMaskTier(recordIndex, maskIndex)) {
    case 0:
        flags = 0;
        for (pairIndex = 0; pairIndex < 6; pairIndex++) {
            flags |= getSaveRecordFlagPair(recordIndex, maskIndex, pairIndex);
        }
        if (!flags) {
            setSaveRecordFlagPair(recordIndex, maskIndex, 0, 3);
            setSaveRecordFlagPair(recordIndex, maskIndex, 1, 3);
            setSaveRecordFlagPair(recordIndex, maskIndex, 2, 3);
            setSaveRecordFlagPair(recordIndex, maskIndex, 3, 3);
            setSaveRecordFlagPair(recordIndex, maskIndex, 4, 3);
        } else {
            setSaveRecordFlagPair(recordIndex, maskIndex, 5, 3);
            setSaveRecordMaskTier(recordIndex, maskIndex, 1);
        }
        break;
    case 1:
        setSaveRecordFlagPair(recordIndex, maskIndex, 6, 3);
        setSaveRecordMaskTier(recordIndex, maskIndex, 2);
        break;
    case 2:
        setSaveRecordFlagPair(recordIndex, maskIndex, 7, 3);
        setSaveRecordMaskTier(recordIndex, maskIndex, 3);
        break;
    case 3:
        setSaveRecordFlagPair(recordIndex, maskIndex, 8, 3);
        setSaveRecordMaskTier(recordIndex, maskIndex, 4);
        break;
    case 4:
        setSaveRecordFlagPair(recordIndex, maskIndex, 0, 0);
        setSaveRecordFlagPair(recordIndex, maskIndex, 1, 0);
        setSaveRecordFlagPair(recordIndex, maskIndex, 2, 0);
        setSaveRecordFlagPair(recordIndex, maskIndex, 3, 0);
        setSaveRecordFlagPair(recordIndex, maskIndex, 4, 0);
        setSaveRecordFlagPair(recordIndex, maskIndex, 5, 0);
        setSaveRecordFlagPair(recordIndex, maskIndex, 6, 0);
        setSaveRecordFlagPair(recordIndex, maskIndex, 7, 0);
        setSaveRecordFlagPair(recordIndex, maskIndex, 8, 0);
        setSaveRecordFlagPair(recordIndex, maskIndex, 9, 0);
        setSaveRecordMaskTier(recordIndex, maskIndex, 0);
        break;
    }
}

void applySaveRecordUnlockOverrides(s16 index) {
    s16 i;
    if (gSaveRecordUnlockOverrideFlags & 1) {
        updatePairedStateFlag(0);
        updatePairedStateFlag(1);
    }
    if (gSaveRecordUnlockOverrideFlags & 2) {
        for (i = 0; i < 10; i++) {
            setSaveRecordMaskTier(index, i, 4);
            cycleSaveRecordUnlockFlags(index, i);
            cycleSaveRecordUnlockFlags(index, i);
            cycleSaveRecordUnlockFlags(index, i);
            cycleSaveRecordUnlockFlags(index, i);
            cycleSaveRecordUnlockFlags(index, i);
            cycleSaveRecordUnlockFlags(index, i);
        }
        gUnlockNotificationFlags = 0x7FF;
    }
    if (gSaveRecordUnlockOverrideFlags & 4) {
        SaveRecord *records = gPlayerSaveRecords;
        records[index].flags |= 0x1FFFC;
    }
    copySaveRecordAndMergeFlags();
}

void drawTextMessagePanel(void) {
    setSpriteColor(128, 128, 128);
    setTexturedQuadColor(128, 128, 128);
    drawWrappedGlyphText16x16(-112, -24, 14, gTextPanelMessage, 8, -1,
                              &D_800EBFF0[D_8013AF78], 3);
    drawDefaultTexturedPanel(-120, -32, 240, 64, 5, 0,
                            &D_800EBFF0[D_8013AF78], 3);
}

void initializeDualResourceTaskAndNotifyParent(void) {
    GameTask *task = D_8013AE88;
    GameTask *parent = D_8013AE98;
    task->data18 = 0;
    parent->data1C++;
    if (task->resource14 != 0xFFFF) {
        while (1) {}
    }
    D_8013AE88->resource14 = allocateTaskMemoryBlock();
    if (D_8013AE88->resource16 != 0xFFFF) {
        while (1) {}
    }
    D_8013AE88->resource16 = allocateTaskMemoryBlock();
    setCurrentTaskCallback(updateMainMenuSelectionTask);
    D_8013AE88->unk10 |= 2;
}
