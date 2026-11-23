#include "common.h"
#include "game/save/save_data.h"
#include "game/engine/game_options.h"
#include "game/engine/byte_search.h"
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

typedef struct ColorCycleResource {
    u8 value, step, minimum, maximum;
} ColorCycleResource;

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

typedef struct RegisteredSelectionPanelResource {
    u8 unk00[0x2C];
    s16 x, y, x2, y2, x3, y3;
    s16 values[6];
    u16 mode;
} RegisteredSelectionPanelResource;

typedef struct AnimatedSpriteResource {
    s16 frame;
    s16 unk02;
    s16 repeats;
    u16 depth;
} AnimatedSpriteResource;

typedef s16 SpriteMotionResource[128];

/* Storage remains in the shared data object. */
typedef struct TaskValueState {
    GameTask *task;
    u16 unk04;
    u16 unk06;
} TaskValueState;

extern s16 gFrameCounter __attribute__((section(".data")));
extern u16 D_8013AF78 __attribute__((section(".data")));
extern u8 D_8013AFCD __attribute__((section(".data")));
extern u8 D_8013AFD3 __attribute__((section(".data")));
extern GameTask *D_8013AE88 __attribute__((section(".data")));
extern GameTask *D_8013AE8C __attribute__((section(".data")));
extern GameTask *D_8013AE98 __attribute__((section(".data")));

extern s16 D_8013B106 __attribute__((section(".data")));

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

/* Storage remains in the shared data object. */
extern TaskValueState D_8012D4A4[3];
extern TimedSequenceEntry D_8012F18C[], D_8012F1AC[];
extern u32 D_8013B3BC;
extern TexturedQuadListEntry *D_8013B3C0[2];
extern GameTask *D_8013B378;
extern GameTask *D_8013B37C;
extern GameTask *D_8013BDA8[4];

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

s16 getTaskTransitionViewportIndex(s32 id);
void createRegisteredSpacedChildTask(s16 childIndex, s16 x, s16 y, TaskCallback callback,
                                     s32 depth, s32 registrationIndex);
void func_80072C54(void);
void updateRegisteredTexturePairTask(void);
void animateRegisteredTexturePairTask(void);

void addTilePacket(s32 x, s32 y, s32 width, s32 height, s32 r, s32 g, s32 b,
                   GsOT *ot, u16 depth);
void updateScrollingBackgroundTask(void);
void drawTaskImagesOnBlackBackground(void);
void startSecondaryDisplayTask(void);
void updateTaskStateTransition(void);
void stopSecondaryDisplayTask(void);
void drawSecondaryDisplayTask(void);
void initTimedSequence(TimedSequence *sequence, const TimedSequenceEntry *entries, s16 index, s16 count);
void updateCountdownTask(void);
void func_8007649C(void);
void updateInputTimedSpriteTask(void);
void updateTimedSequence(TimedSequence *sequence);
void updateTimedSpriteB5Task(void);
void updateValueTransitionTask(void);
void func_80077770(void);
s32 findCharacterSelectionIndex(s16 character);
void func_80078140(void);
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

void configureTaskStateTransition(s32 mode, s32 id, u16 duration) {
    TaskValueState *state;
    s16 index;
    index = getTaskTransitionViewportIndex(id);
    switch (index) {
    case 0: break;
    case 1: id -= 0xFFC; break;
    case 2: id -= 0x1FFC; break;
    }
    state = &D_8012D4A4[index];
    if (duration == 0) state->unk04 = 0xFFFF;
    else state->unk04 = 0xFFFF / duration;
    switch (mode) {
    case 1: case 2: case 4: case 5: case 6: case 7:
    case 11: case 12: case 14: case 15:
        createChildTaskForParentWithReserve(&gGameTasks[4], updateTaskStateTransition);
        if (mode >= 10) D_8013AE8C->data18 = mode - 10;
        else D_8013AE8C->data18 = mode;
        D_8013AE8C->data1A = index;
        D_8013AE8C->data1C = id;
        D_8013AE8C->data1E = 1;
        state->task = D_8013AE8C;
        if (mode < 11) {
            if (duration == 0) {
                if (mode == 1 || mode == 4 || mode == 6) state->unk06 = 0xFFFF;
                else state->unk06 = 0;
            } else {
                if (mode == 1 || mode == 4 || mode == 6) state->unk06 = 0xFFFF;
                else state->unk06 = 0;
            }
        }
        break;
    case 0: case 3: case 8: case 9: case 10: case 13:
    default: state->task = NULL; break;
    }
}

void updateTaskStateTransition(void) {
    u16 index = D_8013AE88->data1A;
    TaskValueState *state = &D_8012D4A4[index];
    GsOT *ot;
    s16 draw;
    GameTask *task;

    if (state->task != D_8013AE88) {
        goto end_task;
    }
    /* configureTaskStateTransition assigns one of the three viewport indices. */
    switch (index) {
    case 0:
        ot = &D_800EBFF0[D_8013AF78];
        draw = 1;
        break;
    case 1:
        ot = &D_800EC018[D_8013AF78];
        draw = shouldRenderPlayerView(0);
        break;
    case 2:
        ot = &D_800EC0B8[D_8013AF78];
        draw = shouldRenderPlayerView(1);
        break;
    }

    if (draw) {
        u16 mode;
        GameTask *drawTask = D_8013AE88;
        mode = drawTask->data18;
        if (mode == 1 || mode == 2 || mode == 6 || mode == 7) {
            drawScreenDarkeningOverlay(state->unk06 >> 8, ot, D_8013AE88->data1C);
        } else if (mode == 4 || mode == 5) {
            drawScreenBrighteningOverlay(state->unk06 >> 8, ot, drawTask->data1C);
        }
    }

    if (D_8013AE88->data1E &&
        (!D_8013AFD3 || D_8013AE88->data18 == 6 || D_8013AE88->data18 == 7)) {
        if ((D_8013AE88->data18 == 1 || D_8013AE88->data18 == 6) && state->unk06 == 0) {
            goto end_task;
        }
        task = D_8013AE88;
        if ((task->data18 == 2 || task->data18 == 7) && state->unk06 == 0xFFFF) {
            goto stop_transition;
        }
        if (D_8013AE88->data18 == 4 && state->unk06 == 0) {
end_task:
            deleteCurrentTaskSubtree();
            return;
        }
        task = D_8013AE88;
        if (task->data18 == 5 && state->unk06 == 0xFFFF) {
stop_transition:
            task->data1E = 0;
            return;
        }
        if (D_8013AE88->data18 == 1 || D_8013AE88->data18 == 4 || D_8013AE88->data18 == 6) {
            if (state->unk06 < state->unk04) {
                state->unk06 = 0;
            } else {
                state->unk06 -= state->unk04;
            }
        }
        if (D_8013AE88->data18 == 2 || D_8013AE88->data18 == 5 || D_8013AE88->data18 == 7) {
            if (0xFFFF - state->unk06 < state->unk04) {
                state->unk06 = 0xFFFF;
            } else {
                state->unk06 += state->unk04;
            }
        }
    }
}

s16 getTaskTransitionViewportIndex(s32 id) {
    if (id >= 0x1000 && id < 0x2000) {
        return 1;
    }
    return 2 * (id >= 0x2000 && id < 0x3000);
}

s32 hasTaskStateValue6(s32 id) {
    return D_8012D4A4[getTaskTransitionViewportIndex(id)].unk06 != 0;
}

s32 isTaskStateTransitionBelowMaximum(s32 id) {
    return D_8012D4A4[getTaskTransitionViewportIndex(id)].unk06 < 0xFFFFu;
}

void clearColorCycleTaskReferences(void) {
    s16 i;
    for (i = 0; i < 4; ) {
        D_8013BDA8[i] = NULL;
        i = (s16)(i + 1);
    }
}

void configureColorCycleTask(s32 mode, s16 minimum, s16 maximum, s32 step, s16 index) {
    GameTask *task;
    ColorCycleResource *resource;
    s32 missing;
    u8 low = minimum;
    u8 high = maximum;
    u8 rate = step;
    if (index < 4) {
        task = D_8013BDA8[index];
        missing = task == NULL;
        if (mode != 0 && (mode < 0 ? 0 : (mode < 5))) {
            if (missing == 1) {
                createChildTaskForParentWithReserve(&gGameTasks[3], updateColorCycleTask);
                task = D_8013BDA8[index] = D_8013AE8C;
                task->data1A = index;
                D_8013AE8C->resource14 = allocateTaskResource();
                resource = (ColorCycleResource *)&D_800D9504[D_8013AE8C->resource14];
                if (mode == 1 || mode == 3) resource->value = minimum;
                else resource->value = maximum;
            } else {
                resource = (ColorCycleResource *)&D_800D9504[task->resource14];
                if (mode == 1) resource->value = minimum;
                if (mode == 2) resource->value = maximum;
            }
            task->data18 = mode;
            if (mode == 3) task->data18 = 1;
            if (mode == 4) task->data18 = 2;
            task->data1E = 0;
            resource->step = rate;
            resource->minimum = low;
            resource->maximum = high;
            D_8013B3A4 = resource->value;
            D_8013B3A5 = resource->value;
            D_8013B3A6 = resource->value;
        } else if (task != NULL) task->data18 = 0;
    }
}

void updateColorCycleTask(void) {
    ColorCycleResource *resource;
    if (D_8013AE88->resource14 == 0xFFFF) { while (1) {} }
    resource = (ColorCycleResource *)&D_800D9504[D_8013AE88->resource14];
    if (D_8013AE88->data18 == 0) {
        D_8013BDA8[D_8013AE88->data1A] = NULL;
        deleteCurrentTaskSubtree();
    } else {
        D_8013AE88->data1E = 0;
        switch (D_8013AE88->data18) {
        case 1:
            if (resource->step >= resource->maximum - resource->value) {
                resource->value = resource->maximum;
                D_8013AE88->data1E = 1;
                D_8013AE88->data18 = 2;
            } else { resource->value += resource->step; }
            break;
        case 2:
            if (resource->step >= resource->value - resource->minimum) {
                resource->value = resource->minimum;
                D_8013AE88->data1E = 1;
                D_8013AE88->data18 = 1;
            } else { resource->value -= resource->step; }
            break;
        }
        D_8013B3A4 = resource->value;
        D_8013B3A5 = resource->value;
        D_8013B3A6 = resource->value;
    }
}

s32 copyColorCycleTaskColor(s16 index, u8 *color) {
    GameTask *task = D_8013BDA8[index];
    if (task == NULL) {
        return 0;
    } else {
        const ColorCycleResource *resource = (const ColorCycleResource *)&D_800D9504[task->resource14];
        color[0] = resource->value;
        color[1] = resource->value;
        color[2] = resource->value;
        return 1;
    }
}

s32 isTaskResourceTransitionComplete(s16 index) {
    GameTask *task;
    task = *(index + D_8013BDA8);
    if (task != NULL && task->data1E == 1) {
        return 1;
    }
    return 0;
}

s32 hasColorCycleTaskReferences(void) {
    s32 found = 0;
    s16 i = 0;

    do {
        if (D_8013BDA8[i] != NULL) {
            found = 1;
        }
    } while (++i < 4);
    return found;
}

void drawScreenDarkeningOverlay(s32 intensity, GsOT *ot, u16 depth) {
    TILE *packet = (TILE *)GsGetWorkBase();
    u16 tpage;
    SetTile(packet);
    setXY0(packet, -170, -130);
    setWH(packet, 340, 260);
    setRGB0(packet, intensity, intensity, intensity);
    SetSemiTrans(packet, 1);
    AddPrim(ot->org + depth, packet);
    tpage = GetTPage(0, 2, 0, 0);
    SetDrawMode((DR_MODE *)(packet + 1), 1, 1, tpage, NULL);
    AddPrim(ot->org + depth, ++packet);
    func_800BF2C8((u32)((DR_MODE *)packet + 1));
}

void drawScreenBrighteningOverlay(s32 intensity, GsOT *ot, u16 depth) {
    TILE *packet = (TILE *)GsGetWorkBase();
    u16 tpage;
    SetTile(packet);
    setXY0(packet, -170, -130);
    setWH(packet, 340, 260);
    setRGB0(packet, intensity, intensity, intensity);
    SetSemiTrans(packet, 1);
    AddPrim(ot->org + depth, packet);
    tpage = GetTPage(0, 1, 0, 0);
    SetDrawMode((DR_MODE *)(packet + 1), 1, 1, tpage, NULL);
    AddPrim(ot->org + depth, ++packet);
    func_800BF2C8((u32)((DR_MODE *)packet + 1));
}

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
                existing = (ScrollingBackgroundResource *)&D_800D9504[D_8013B378->resource14];
                existing->stepX = value1;
                existing->stepY = value2;
                return;
            }
            D_8013B378->data18 = 0;
        }
        createChildTaskForParentWithReserve(&gGameTasks[3], updateScrollingBackgroundTask);
        D_8013AE8C->resource14 = allocateTaskResource();
        resource = (ScrollingBackgroundResource *)&D_800D9504[D_8013AE8C->resource14];
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
    resource = (ScrollingBackgroundResource *)&D_800D9504[D_8013AE88->resource14];
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
        D_8013AE8C->resource14 = allocateTaskResource();
        initTimedSequence((TimedSequence *)&D_800D9504[D_8013AE8C->resource14], gSecondaryTaskSequenceEntries, 0, 6);
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
        func_8006EF30(36, 88, 15, gSecondaryDisplayLabel, 15, -1,
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

/* DECOMP_ATTEMPT: inline 72/77 */
#ifdef NON_MATCHING
void func_80071EF8(s16 x0, s16 y0, s32 width, s32 height, s16 textureDepth, s16 textureX, s16 textureY, s16 clutX, s16 clutY, GsOT *ot, u16 depth) {
    POLY_FT4 *quad = &gTexturedQuadTemplate;
    s32 x, y;
    s32 right;
    quad->clut = GetClut(clutX, clutY);
    x = x0;
    setRGB0(quad, 128, 128, 128);
    quad->tpage = GetTPage(textureDepth, 1, textureX, textureY);
    y = y0;
    quad->y0 = y;
    quad->y1 = y;
    right = width + x;
    y = height + y;
    quad->x0 = x;
    quad->x1 = right;
    quad->x2 = x;
    quad->y2 = y;
    quad->x3 = right;
    quad->y3 = y;
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
#else
INCLUDE_ASM("asm/nonmatchings/menu/task_widgets", func_80071EF8);
#endif

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

/* DECOMP_ATTEMPT: inline 81/84 */
#ifdef NON_MATCHING
void func_8007240C(s16 x, s16 y, s16 width, s16 height, u8 r, u8 g, u8 b,
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
    packet++;
    SetDrawMode((DR_MODE *)packet, 1, 1, tpage, NULL);
    AddPrim(ot->org + depth, packet);
    func_800BF2C8((u32)((DR_MODE *)packet + 1));
}
#else
INCLUDE_ASM("asm/nonmatchings/menu/task_widgets", func_8007240C);
#endif

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
    RegisteredTaskEntry *entries = (RegisteredTaskEntry *)&D_800D9504[resource];
    return &entries[index / 10];
}

void createRegisteredSpacedChildTasks(s16 mode, u32 acceptMask, u32 cancelMask,
                                      const u8 *data44, const u8 *data48,
                                      const u8 *data4C, u8 arg6, s8 arg7, u8 arg8,
                                      s16 x, s16 y, s16 spacingX, s16 spacingY,
                                      s16 count, s16 arg14, TaskCallback callback3C,
                                      TaskCallback callback40, s32 value, s32 index) {
    SpacedGroupResource *state;
    RegisteredTaskEntry *entry;
    if (count != 0) {
        createChildTaskWithReserve(initializeSpacedChildTasks);
        D_8013AE8C->data1E = index;
        D_8013AE8C->resource14 = allocateTaskResource();
        state = (SpacedGroupResource *)&D_800D9504[D_8013AE8C->resource14];
        state->mode = mode;
        state->spacingX = spacingX;
        state->spacingY = spacingY;
        state->count = count;
        state->unk5C = arg6;
        state->unk5E = arg7;
        state->unk5D = arg8;
        state->value = value;
        state->unk60 = arg14;
        state->callback3C = callback3C;
        state->callback40 = callback40;
        state->data44 = data44;
        state->data48 = data48;
        state->data4C = data4C;
        state->acceptMask = acceptMask;
        state->cancelMask = cancelMask;
        state->r = 128;
        state->g = 128;
        state->b = 128;
        initMovement2D(&state->movement, x << 12, y << 12);
        D_8013AE8C->resource16 = allocateTaskResource();
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
    state = (SpacedTaskResource *)&D_800D9504[D_8013AE88->resource14];
    if ((u16)(D_8013AE88->data1E % 10U) == 0) {
        if (D_8013AE98->resource14 == 0xFFFF) { while (1) {} }
        entries = (RegisteredTaskEntry *)&D_800D9504[D_8013AE98->resource14];
        entry = &entries[D_8013AE88->data1E / 10U];
    } else {
        if (D_8013AE98->resource16 == 0xFFFF) { while (1) {} }
        entries = (RegisteredTaskEntry *)&D_800D9504[D_8013AE98->resource16];
        entry = &entries[D_8013AE88->data1E / 10U];
    }
    for (i = 0; i < state->count; i++) {
        createRegisteredSpacedChildTask(i, state->x / 4096 + state->spacingX * i,
                                        state->y / 4096 + state->spacingY * i,
                                        state->callback, state->value, i * 10 + 1);
    }
    entry->flags = 0;
    setCurrentTaskCallback(func_80072C54);
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
    D_8013AE8C->resource14 = allocateTaskResource();
    initMovement2D((Movement2D *)&D_800D9504[D_8013AE8C->resource14], x << 12, y << 12);
    D_8013AE8C->resource16 = allocateTaskResource();
    color = (ByteTriplet *)&D_800D9504[D_8013AE8C->resource16];
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

/* DECOMP_ATTEMPT: inline 639/671 */
#ifdef NON_MATCHING
typedef struct SpacedGroupSelection {
    s16 mode, selectedIndex, previousIndex, choiceResult;
} SpacedGroupSelection;

typedef struct SelectionGroupState {
    Movement2D movement;
    u32 acceptMask, cancelMask;
    SpacedGroupSelection selection;
    u8 unk38[4];
    TaskCallback callback3C, callback40;
    const u8 *data44, *data48, *data4C;
    s32 value;
    s16 mode, spacingX, spacingY, count;
    u8 unk5C, unk5D;
    s8 unk5E;
    u8 unk5F;
    s16 unk60;
    u8 r, g, b;
} SelectionGroupState;

typedef struct SpacedGroupSounds {
    u8 bank;
    u16 programAndTone;
} SpacedGroupSounds;

typedef struct SpacedGroupConfirmSound {
    s16 sprite;
    u8 color;
    u8 bank;
    u16 programAndTone;
} SpacedGroupConfirmSound;

void func_80072C54(void) {
    u16 input = 0;
    SelectionGroupState *state;
    RegisteredTaskEntry *children;
    RegisteredTaskEntry *entry;
    RegisteredTaskEntry *entries;
    SpacedGroupSelection *selection;
    s16 i;
    CVECTOR *color;
    const SpacedGroupConfirmSound *confirm;

    if (D_8013AE88->resource14 == 0xFFFF) { while (1) {} }
    state = (SelectionGroupState *)&D_800D9504[D_8013AE88->resource14];
    if (D_8013AE88->resource16 == 0xFFFF) { while (1) {} }
    children = (RegisteredTaskEntry *)&D_800D9504[D_8013AE88->resource16];
    if ((u16)(D_8013AE88->data1E % 10U) == 0) {
        if (D_8013AE98->resource14 == 0xFFFF) { while (1) {} }
        entries = (RegisteredTaskEntry *)&D_800D9504[D_8013AE98->resource14];
        entry = &entries[D_8013AE88->data1E / 10U];
    } else {
        if (D_8013AE98->resource16 == 0xFFFF) { while (1) {} }
        entries = (RegisteredTaskEntry *)&D_800D9504[D_8013AE98->resource16];
        entry = &entries[D_8013AE88->data1E / 10U];
    }
    if (D_8013AE98->resource14 == 0xFFFF) { while (1) {} }
    selection = (SpacedGroupSelection *)&D_800D9504[D_8013AE98->resource14];
    state->selection.mode = selection->mode;
    state->selection.selectedIndex = selection->selectedIndex;
    state->selection.previousIndex = selection->previousIndex;
    state->selection.choiceResult = selection->choiceResult;

    switch (entry->command) {
    case 1:
        for (i = 0; i < state->count; i++) {
            GameTask *child = children[i].task;
            if (child != NULL) {
                if (i == state->selection.selectedIndex) {
                    color = (CVECTOR *)&D_800D9504[child->resource16];
                    color->r = 128;
                    color->g = 128;
                    color->b = 128;
                } else {
                    color = (CVECTOR *)&D_800D9504[child->resource16];
                    color->r = 64;
                    color->g = 64;
                    color->b = 64;
                }
            }
        }
        goto clearCommand;
    case 2:
        for (i = 0; i < state->count; i++) {
            GameTask *child = children[i].task;
            if (child != NULL) {
                color = (CVECTOR *)&D_800D9504[child->resource16];
                color->r = 128;
                color->g = 128;
                color->b = 128;
            }
        }
        goto clearCommand;
    case 3:
        for (i = 0; i < state->count; i++) {
            GameTask *child = children[i].task;
            if (child != NULL) {
                color = (CVECTOR *)&D_800D9504[child->resource16];
                color->r = 64;
                color->g = 64;
                color->b = 64;
            }
        }
        goto clearCommand;
    case 10:
        if (state->unk5D != 0) {
            D_8013AE88->data18 = state->unk5D;
            entry->command = 11;
            entry->flags = 1;
        }
        break;
    case 11:
        if (D_8013AE88->data18 == 0) {
            GameTask *child = children[state->selection.selectedIndex].task;
            if (child != NULL) {
                color = (CVECTOR *)&D_800D9504[child->resource16];
                color->r = 128;
                color->g = 128;
                color->b = 128;
            }
            goto clearCommand;
        }
        if ((D_8013AE88->data18 & 1) == 0) {
            GameTask *child = children[state->selection.selectedIndex].task;
            if (child != NULL) {
                color = (CVECTOR *)&D_800D9504[child->resource16];
                color->r = 128;
                color->g = 128;
                color->b = 128;
            }
        } else {
            GameTask *child = children[state->selection.selectedIndex].task;
            if (child != NULL) {
                color = (CVECTOR *)&D_800D9504[child->resource16];
                color->r = 64;
                color->g = 64;
                color->b = 64;
            }
        }
        D_8013AE88->data18--;
        entry->flags = 1;
        break;
    case 20:
        state->selection.mode = 1;
        goto clearCommand;
    case 21:
        state->selection.mode = 2;
        goto clearCommand;
    case 22:
        state->selection.mode = 3;
        goto clearCommand;
    case 23:
        state->selection.mode = 0;
clearCommand:
        entry->command = 0;
        /* fall through */
    case 0:
        entry->flags = 0;
        break;
    case 99:
    default:
        entry->task = NULL;
        deleteCurrentTaskSubtree();
        return;
    }

    for (i = 0; i < state->count; i++) {
        GameTask *child = children[i].task;
        if (child != NULL) {
            entry->flags |= updateMovement2D((Movement2D *)&D_800D9504[child->resource14]);
        }
    }
    copyColorCycleTaskColor(0, &state->r);
    if ((u32)((u16)entry->command - 1) >= 3 && entry->command != 11) {
        for (i = 0; i < state->count; i++) {
            GameTask *child = children[i].task;
            if (child != NULL) {
                color = (CVECTOR *)&D_800D9504[child->resource16];
                if (i == state->selection.selectedIndex) {
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

    if (state->selection.choiceResult == 0 && state->selection.mode > 0) {
        if (state->mode & 1) input |= D_8013AEE8[0];
        if (state->mode & 2) input |= D_8013AEE8[1];
        i = state->selection.selectedIndex;
        if (state->unk60 >= 0) {
            if (input & 0x1000) {
                i--;
                if (i < 0) {
                    i = 0;
                    if (state->unk5E == 1) i = state->count - 1;
                }
            }
            if (input & 0x4000) {
                i++;
                if (i >= state->count) {
                    if (state->unk5E == 1) i = 0;
                    else i = state->count - 1;
                }
            }
        } else {
            if (input & 0x8000) {
                i--;
                if (i < 0) {
                    i = 0;
                    if (state->unk5E == 1) i = state->count - 1;
                }
            }
            if (input & 0x2000) {
                i++;
                if (i >= state->count) {
                    if (state->unk5E == 1) i = 0;
                    else i = state->count - 1;
                }
            }
            if (input & 0x1000) {
                if (i == state->count - 1) i--;
                else if (i >= 4) i -= 4;
                if (i < 0) {
                    i = 0;
                    if (state->unk5E == 1) i = state->count - 1;
                }
            }
            if (input & 0x4000) {
                if (i < 4) {
                    i += 4;
                } else {
                    if (i < state->count - 1) i = state->count - 1;
                    else i += 4;
                }
                if (i >= state->count) {
                    if (state->unk5E == 1) i = 0;
                    else i = state->count - 1;
                }
            }
        }
        if (D_8013B108 == 3 && !isControllerInputSupported(1)) i = 0;
        if (i != state->selection.selectedIndex) {
            state->selection.previousIndex = state->selection.selectedIndex;
            state->selection.selectedIndex = i;
            if (state->data4C != NULL) {
                const SpacedGroupSounds *sound = (const SpacedGroupSounds *)state->data4C;
                if (sound->bank != 0xFF) {
                    playSoundEffect(sound->bank, sound->programAndTone, 0x20);
                }
            } else {
                playSoundEffect(0, 0x100, 0x20);
            }
        }
        if (input & state->acceptMask) {
            state->selection.choiceResult = 1;
            state->selection.mode = 3;
            confirm = (const SpacedGroupConfirmSound *)getMappedGridSpriteEntry(
                (const GridSpriteEntry *)state->data44, state->data48, state->selection.selectedIndex);
            if (confirm->bank != 0xFF) playSoundEffect(confirm->bank, confirm->programAndTone, 0x20);
        }
        if (input & state->cancelMask) {
            state->selection.choiceResult = -1;
            if (state->data4C != NULL) {
                const SpacedGroupSounds *sound = &((const SpacedGroupSounds *)state->data4C)[1];
                if (sound->bank != 0xFF) {
                    playSoundEffect(sound->bank, sound->programAndTone, 0x20);
                }
            } else {
                playSoundEffect(0, 0, 0x20);
            }
        }
    }

    if (state->callback3C != NULL) state->callback3C();
    state = (SelectionGroupState *)&D_800D9504[D_8013AE88->resource14];
    selection->mode = state->selection.mode;
    selection->selectedIndex = state->selection.selectedIndex;
    selection->previousIndex = state->selection.previousIndex;
    selection->choiceResult = state->selection.choiceResult;
}
#else
INCLUDE_ASM("asm/nonmatchings/menu/task_widgets", func_80072C54);
#endif

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
    const SpacedGroupResource *resource = (const SpacedGroupResource *)&D_800D9504[resourceIndex];
    const u8 *mapping = resource->data48;
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
    state = (TaskGridState *)&D_800D9504[task->resource14];
    entries = (TaskGridEntry *)&D_800D9504[task->resource16];
    if (mode == 1) {
        i = state->selectedIndex;
        end = i + 1;
    } else {
        i = 0;
        end = state->count;
    }
    for (; i < end; i++) {
        if (mode == 2 && i == state->selectedIndex) continue;
        configureMovement2D((Movement2D *)&D_800D9504[entries[i].task->resource14], x, y, stepX, stepY, delay);
    }
}

void moveTaskChildrenWithSpacing(s32 mode, GameTask *task, s32 x, s32 y, s32 speedX, s32 speedY, s32 delay) {
    ChildMovementGroup *state;
    SelectedSpriteChild *children;
    s16 i, end;
    Movement2D *movement;
    state = (ChildMovementGroup *)&D_800D9504[task->resource14];
    children = (SelectedSpriteChild *)&D_800D9504[task->resource16];
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
        movement = (Movement2D *)&D_800D9504[children[i].task->resource14];
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
    state = (TaskGridState *)&D_800D9504[task->resource14];
    entries = (TaskGridEntry *)&D_800D9504[task->resource16];
    if (mode == 1) {
        i = state->selectedIndex;
        end = i + 1;
    } else {
        i = 0;
        end = state->count;
    }
    for (; i < end; i++) {
        if (mode == 2 && i == state->selectedIndex) continue;
        movement = (Movement2D *)&D_800D9504[entries[i].task->resource14];
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
    state = (TaskGridState *)&D_800D9504[task->resource14];
    entries = (TaskGridEntry *)&D_800D9504[task->resource16];
    if (mode == 1) {
        i = state->selectedIndex;
        end = i + 1;
    } else {
        i = 0;
        end = state->count;
    }
    for (; i < end; i++) {
        if (mode == 2 && i == state->selectedIndex) continue;
        initMovement2D((Movement2D *)&D_800D9504[entries[i].task->resource14], x + spacingX * i, y + spacingY * i);
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
    state = (SelectedSpriteResource *)&D_800D9504[D_8013AE88->resource14];
    if (D_8013AE88->resource16 == 0xFFFF) {
        while (1) {}
    }
    children = (SelectedSpriteChild *)&D_800D9504[D_8013AE88->resource16];
    mode = state->mode;
    if ((s16)mode > 0) {
        if ((u32)(mode - 1) < 2) {
            setSpriteColor(state->r, state->g, state->b);
        } else {
            setSpriteColor(128, 128, 128);
        }
        position = (Movement2D *)&D_800D9504[children[state->selectedIndex].task->resource14];
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
    position = (Movement2D *)&D_800D9504[D_8013AE88->resource14];
    if (D_8013AE88->resource16 == 0xFFFF) { for (;;) {} }
    color = (ByteTriplet *)&D_800D9504[D_8013AE88->resource16];
    if (D_8013AE98->resource14 == 0xFFFF) { for (;;) {} }
    parent = (GridSpriteParent *)&D_800D9504[D_8013AE98->resource14];
    if ((u16)(D_8013AE88->data1E % 10U) == 0) {
        if (D_8013AE98->resource14 == 0xFFFF) { for (;;) {} }
        entries = (RegisteredTaskEntry *)&D_800D9504[D_8013AE98->resource14];
        entry = &entries[D_8013AE88->data1E / 10U];
    } else {
        if (D_8013AE98->resource16 == 0xFFFF) { for (;;) {} }
        entries = (RegisteredTaskEntry *)&D_800D9504[D_8013AE98->resource16];
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
    D_8013AE8C->resource14 = allocateTaskResource();
    initMovement2D((Movement2D *)&D_800D9504[D_8013AE8C->resource14], x << 12, y << 12);
    D_8013AE8C->resource16 = allocateTaskResource();
    resource = (RegisteredTextResource *)&D_800D9504[D_8013AE8C->resource16];
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
    movement = (Movement2D *)&D_800D9504[D_8013AE88->resource14];
    if (D_8013AE88->resource16 == 0xFFFF) { while (1) {} }
    resource = (RegisteredTextResource *)&D_800D9504[D_8013AE88->resource16];
    if ((u16)(D_8013AE88->data1E % 10U) == 0) {
        if (D_8013AE98->resource14 == 0xFFFF) { while (1) {} }
        entries = (RegisteredTaskEntry *)&D_800D9504[D_8013AE98->resource14];
        entry = &entries[D_8013AE88->data1E / 10U];
    } else {
        if (D_8013AE98->resource16 == 0xFFFF) { while (1) {} }
        entries = (RegisteredTaskEntry *)&D_800D9504[D_8013AE98->resource16];
        entry = &entries[D_8013AE88->data1E / 10U];
    }
    if (D_8013AE98->resource14 == 0xFFFF) { while (1) {} }
    selection = (SelectionEntry *)&D_800D9504[D_8013AE98->resource14];
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
            setSpriteColor(gTaskWidgetRedAbsolute, gTaskWidgetGreenAbsolute, gTaskWidgetBlueAbsolute);
        } else {
            setSpriteColor(128, 128, 128);
        }

        if (resource->text != 0) {
            func_8006EC84((s16)(movement->x / 4096 + 4), (s16)(movement->y / 4096 + 4),
                          14, resource->text, 7, -1, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
        } else if (resource->texts != 0) {
            s16 textIndex = mapMenuByteIndex(resource->mapping, selection->selected);

            func_8006EC84((s16)(movement->x / 4096 + 4), (s16)(movement->y / 4096 + 4),
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
    TaskResourceRecord *resource = &D_800D9504[index];
    resource->unk08 = data;
}

s16 hasTaskResource16Value(GameTask *task) {
    TaskResourceRecord *resource = task->resource16 + D_800D9504;
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
    D_8013AE8C->resource14 = allocateTaskResource();
    initMovement2D((Movement2D *)&D_800D9504[D_8013AE8C->resource14], x << 12, y << 12);
    D_8013AE8C->resource16 = allocateTaskResource();
    resource = (CallbackResource *)&D_800D9504[D_8013AE8C->resource16];
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
    movement = (Movement2D *)&D_800D9504[D_8013AE88->resource14];
    if (D_8013AE88->resource16 == 0xFFFF) { while (1) {} }
    resource = (CallbackResource *)&D_800D9504[D_8013AE88->resource16];
    if ((u16)(D_8013AE88->data1E % 10U) == 0) {
        if (D_8013AE98->resource14 == 0xFFFF) { while (1) {} }
        entries = (RegisteredTaskEntry *)&D_800D9504[D_8013AE98->resource14];
        entry = &entries[D_8013AE88->data1E / 10U];
    } else {
        if (D_8013AE98->resource16 == 0xFFFF) { while (1) {} }
        entries = (RegisteredTaskEntry *)&D_800D9504[D_8013AE98->resource16];
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
            setSpriteColor(gTaskWidgetRedAbsolute, gTaskWidgetGreenAbsolute, gTaskWidgetBlueAbsolute);
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
    parent = (TexturePairParent *)&D_800D9504[D_8013AE88->resource14];
    createChildTaskWithReserve(updateRegisteredTexturePairTask);
    D_8013AE8C->data1A = 0;
    D_8013AE8C->data1C = depth;
    D_8013AE8C->data1E = index;
    D_8013AE8C->resource14 = allocateTaskResource();
    initMovement2D((Movement2D *)&D_800D9504[D_8013AE8C->resource14], x << 12, y << 12);
    D_8013AE8C->resource16 = allocateTaskResource();
    resource = (TexturePairResource *)&D_800D9504[D_8013AE8C->resource16];
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
    movement = (Movement2D *)&D_800D9504[D_8013AE88->resource14];
    if (D_8013AE88->resource16 == 0xFFFF) { while (1) {} }
    resource = (TexturePairResource *)&D_800D9504[D_8013AE88->resource16];
    if ((u16)(D_8013AE88->data1E % 10U) == 0) {
        if (D_8013AE98->resource14 == 0xFFFF) { while (1) {} }
        entries = (RegisteredTaskEntry *)&D_800D9504[D_8013AE98->resource14];
        entry = &entries[D_8013AE88->data1E / 10U];
    } else {
        if (D_8013AE98->resource16 == 0xFFFF) { while (1) {} }
        entries = (RegisteredTaskEntry *)&D_800D9504[D_8013AE98->resource16];
        entry = &entries[D_8013AE88->data1E / 10U];
    }
    if (D_8013AE98->resource14 == 0xFFFF) { while (1) {} }
    parent = (TexturePairParent *)&D_800D9504[D_8013AE98->resource14];
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
    movement = (Movement2D *)&D_800D9504[D_8013AE88->resource14];
    if (D_8013AE88->resource16 == 0xFFFF) { while (1) {} }
    resource = (TexturePairResource *)&D_800D9504[D_8013AE88->resource16];
    if (D_8013AE98->resource14 == 0xFFFF) { while (1) {} }
    parent = (TexturePairParent *)&D_800D9504[D_8013AE98->resource14];
    if ((u16)(D_8013AE88->data1E % 10U) == 0) {
        if (D_8013AE98->resource14 == 0xFFFF) { while (1) {} }
        entries = (RegisteredTaskEntry *)&D_800D9504[D_8013AE98->resource14];
        entry = &entries[D_8013AE88->data1E / 10U];
    } else {
        if (D_8013AE98->resource16 == 0xFFFF) { while (1) {} }
        entries = (RegisteredTaskEntry *)&D_800D9504[D_8013AE98->resource16];
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
    D_8013AE8C->resource14 = allocateTaskResource();
    resource = (ValueTransitionResource *)&D_800D9504[D_8013AE8C->resource14];
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
    resource = (ValueTransitionResource *)&D_800D9504[D_8013AE88->resource14];
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
        if (D_8013AEE8[0] != 0 || D_8013AEE8[1] != 0) {
            value = resource->target;
            D_8013AE88->data1A = 1;
        }
        *resource->value = value;
    }
}

GameTask *createCountdownTask(u32 frames) {
    GameTask *task;
    TaskResourceRecord *resource;

    createChildTaskWithReserve(updateCountdownTask);
    D_8013AE8C->data18 = 0;
    D_8013AE8C->data1A = 0;
    D_8013AE8C->resource14 = allocateTaskResource();
    task = D_8013AE8C;
    resource = &D_800D9504[task->resource14];
    resource->unk04 = frames;
    return task;
}

void updateCountdownTask(void) {
    TaskResourceRecord *resource;

    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = &D_800D9504[D_8013AE88->resource14];
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
                                     s16 x2, s16 y2, s16 x3, s16 y3, s32 depth, s32 index) {
    RegisteredSelectionPanelResource *resource;
    ValuePanelEntry *entry;
    createChildTaskWithReserve(func_8007649C);
    D_8013AE8C->data18 = 0;
    D_8013AE8C->data1A = mode & 0xFF;
    D_8013AE8C->data1C = depth;
    D_8013AE8C->data1E = index;
    D_8013AE8C->resource14 = allocateTaskResource();
    resource = (RegisteredSelectionPanelResource *)&D_800D9504[D_8013AE8C->resource14];
    resource->x = x;
    resource->y = y;
    resource->x2 = x2;
    resource->y2 = y2;
    resource->x3 = x3;
    resource->y3 = y3;
    resource->values[0] = values[0];
    resource->values[1] = values[1];
    resource->values[2] = values[2];
    resource->values[3] = values[3];
    resource->values[4] = values[4];
    resource->values[5] = values[5];
    resource->mode = (mode & 0xFFFF) >> 8;
    D_8013AE8C->resource16 = allocateTaskResource();
    if (index % 10 == 0) {
        ValuePanelEntry *table;
        if (D_8013AE88->resource14 == 0xFFFF) { while (1) {} }
        table = (ValuePanelEntry *)&D_800D9504[D_8013AE88->resource14];
        entry = table + index / 10;
    } else {
        ValuePanelEntry *table;
        if (D_8013AE88->resource16 == 0xFFFF) { while (1) {} }
        table = (ValuePanelEntry *)&D_800D9504[D_8013AE88->resource16];
        entry = table + index / 10;
    }
    entry->flags = 1;
    entry->active = -1;
    entry->task = D_8013AE8C;
}

/* DECOMP_ATTEMPT: inline 424/481 */
#ifdef NON_MATCHING
typedef struct MovingPanelState {
    s16 highlight;
    s16 selection;
    s16 respondingControllers;
    s16 closing;
} MovingPanelState;

typedef struct MovingPanelResource {
    Movement2D movement;
    u8 color[3];
    u8 pad2B;
    s16 x, y;
    s16 spacingX, spacingY;
    s16 targetX, targetY;
    s16 values[6];
    u16 mode;
} MovingPanelResource;

void func_8007649C(void) {
    MovingPanelResource *panel;
    MovingPanelState *state;
    ValuePanelEntry *entry;
    u16 input;
    s16 selection;
    s16 x, y;
    s32 ready;
    s32 color;

    input = 0;
    if (D_8013AE88->resource14 == 0xFFFF) { while (1) {} }
    panel = (MovingPanelResource *)&D_800D9504[D_8013AE88->resource14];
    if (D_8013AE88->resource16 == 0xFFFF) { while (1) {} }
    state = (MovingPanelState *)&D_800D9504[D_8013AE88->resource16];

    if ((u16)(D_8013AE88->data1E % 10U) == 0) {
        ValuePanelEntry *table;
        if (D_8013AE98->resource14 == 0xFFFF) { while (1) {} }
        table = (ValuePanelEntry *)&D_800D9504[D_8013AE98->resource14];
        entry = table + D_8013AE88->data1E / 10U;
    } else {
        ValuePanelEntry *table;
        if (D_8013AE98->resource16 == 0xFFFF) { while (1) {} }
        table = (ValuePanelEntry *)&D_800D9504[D_8013AE98->resource16];
        entry = table + D_8013AE88->data1E / 10U;
    }

    switch (entry->flags) {
    case 1:
        initMovement2D(&panel->movement, panel->x << 12, panel->y << 12);
        configureMovement2D(&panel->movement, panel->targetX << 12, panel->targetY << 12, 0x8000, 0x8000, 0);
        state->highlight = 0;
        state->selection = panel->mode;
        state->closing = 0;
        goto advance;
    case 2:
        if (updateMovement2D(&panel->movement) == 0) {
            configureColorCycleTask(1, 0, 128, 8, 0);
            D_8013AE88->data18 = 0;
            state->highlight = 1;
            state->respondingControllers = 0;
            goto advance;
        }
        break;
    case 3:
        ready = isTaskResourceTransitionComplete(0);
        if (ready == 1 && D_8013AE88->data18 == 0) {
            configureColorCycleTask(2, 64, 128, 4, 0);
            state->highlight = 2;
            D_8013AE88->data18 = ready;
        }
        if (D_8013AE88->data1A & 1) input |= D_8013AEE8[0];
        if (D_8013AE88->data1A & 2) input |= D_8013AEE8[1];

        selection = state->selection;
        if (input & 0x1000) selection--;
        if (input & 0x4000) selection++;
        if (selection < 0) selection = 0;
        if (selection >= 2) selection = 1;
        if (state->selection != selection) {
            state->selection = selection;
            playSoundEffect(0, 0x100, 32);
        }
        if (input & 0xB0) {
            state->closing = 1;
            playSoundEffect(0, 0, 32);
            if ((D_8013AE88->data1A & 1) && (D_8013AEE8[0] & 0xB0)) {
                state->respondingControllers |= 1;
            }
            if ((D_8013AE88->data1A & 2) && (D_8013AEE8[1] & 0xB0)) {
                state->respondingControllers |= 2;
            }
        }
        if (input & 0x40) {
            state->selection = 2;
            state->highlight = 0;
            state->closing = 1;
            playSoundEffect(0, 0, 32);
            if ((D_8013AE88->data1A & 1) && (D_8013AEE8[0] & 0x40)) {
                state->respondingControllers |= 1;
            }
            if ((D_8013AE88->data1A & 2) && (D_8013AEE8[1] & 0x40)) {
                state->respondingControllers |= 2;
            }
        }
        if (state->closing == 1) {
            configureMovement2D(&panel->movement, panel->x << 12, panel->y << 12, 0x8000, 0x8000, -1000);
            state->highlight = 0;
            goto advance;
        }
        break;
    case 4:
        if (updateMovement2D(&panel->movement) == 0) {
advance:
            entry->active = -1;
            entry->flags++;
        }
        break;
    default:
finish:
        entry->active = state->selection;
        entry->flags = state->respondingControllers;
        deleteCurrentTaskSubtree();
        return;
    case 99:
        goto finish;
    }

    x = panel->movement.x / 4096;
    y = panel->movement.y / 4096;
    if (state->highlight > 0) {
        if ((u16)state->highlight == 1 || (u16)state->highlight == 2) {
            copyColorCycleTaskColor(0, panel->color);
            setSpriteColor(panel->color[0], panel->color[1], panel->color[2]);
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
#else
INCLUDE_ASM("asm/nonmatchings/menu/task_widgets", func_8007649C);
#endif

void createInputTimedSpriteTask(s32 x, s32 y, const TimedSequenceEntry *entries, s32 count,
    s32 controllers, u32 acceptMask, u32 cancelMask, s32 depth, s32 index) {
    InputTimedSpriteResource *resource;
    ValuePanelEntry *entry;

    createChildTaskWithReserve(updateInputTimedSpriteTask);
    D_8013AE8C->data1C = depth;
    D_8013AE8C->data1E = index;
    D_8013AE8C->resource14 = allocateTaskResource();
    resource = (InputTimedSpriteResource *)&D_800D9504[D_8013AE8C->resource14];
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
        table = (ValuePanelEntry *)&D_800D9504[D_8013AE88->resource14];
        entry = table + index / 10;
    } else {
        ValuePanelEntry *table;
        if (D_8013AE88->resource16 == 0xFFFF) {
            while (1) {}
        }
        table = (ValuePanelEntry *)&D_800D9504[D_8013AE88->resource16];
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
    state = (InputTimedSpriteResource *)&D_800D9504[D_8013AE88->resource14];
    if ((u16)(D_8013AE88->data1E % 10U) == 0) {
        if (D_8013AE98->resource14 == 0xFFFF) { while (1) {} }
        entries = (ValuePanelEntry *)&D_800D9504[D_8013AE98->resource14];
        entry = &entries[D_8013AE88->data1E / 10U];
    } else {
        if (D_8013AE98->resource16 == 0xFFFF) { while (1) {} }
        entries = (ValuePanelEntry *)&D_800D9504[D_8013AE98->resource16];
        entry = &entries[D_8013AE88->data1E / 10U];
    }
    if (entry->flags == 99) {
        entry->active = 0;
        deleteCurrentTaskSubtree();
    } else {
        if (state->controllers & 1) input |= D_8013AEE8[0];
        if (state->controllers & 2) input |= D_8013AEE8[1];
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
    TaskResourceRecord *resource;
    createChildTaskWithReserve(updateTimedSpriteB5Task);
    D_8013AE8C->data18 = 0;
    D_8013AE8C->data1A = 0;
    D_8013AE8C->resource14 = allocateTaskResource();
    initMovement2D((Movement2D *)&D_800D9504[D_8013AE8C->resource14], x << 12, y << 12);
    D_8013AE8C->resource16 = allocateTaskResource();
    task = D_8013AE8C;
    resource = &D_800D9504[task->resource16];
    resource->unk04 = frames;
    return task;
}

void updateTimedSpriteB5Task(void) {
    Movement2D *movement;
    TaskResourceRecord *lifetime;
    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    movement = (Movement2D *)&D_800D9504[D_8013AE88->resource14];
    if (D_8013AE88->resource16 == 0xFFFF) {
        while (1) {}
    }
    lifetime = &D_800D9504[D_8013AE88->resource16];
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
    D_8013AE8C->resource14 = allocateTaskResource();
    initMovement2D((Movement2D *)&D_800D9504[D_8013AE8C->resource14], x << 12, y << 12);
    D_8013AE8C->resource16 = allocateTaskResource();
    task = D_8013AE8C;
    resource = (AnimatedSpriteResource *)&D_800D9504[task->resource16];
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
    movement = (Movement2D *)&D_800D9504[D_8013AE88->resource14];
    if (D_8013AE88->resource16 == 0xFFFF) {
        while (1) {}
    }
    sprite = (AnimatedSpriteResource *)&D_800D9504[D_8013AE88->resource16];
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
    D_8013AE88->resource14 = allocateTaskResource();
    if (D_8013AE88->resource16 != 0xFFFF) {
        while (1) {}
    }
    D_8013AE88->resource16 = allocateTaskResource();
    setCurrentTaskCallback(func_80077770);
    D_8013AE88->unk10 |= 2;
}

/* DECOMP_ATTEMPT: unattempted */
INCLUDE_ASM("asm/nonmatchings/menu/task_widgets", func_80077770);

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
    selection = (SelectionEntry *)&D_800D9504[D_8013AE88->resource14];
    createChildTaskWithReserve(func_80078140);
    D_8013AE8C->data1A = 0;
    D_8013AE8C->data1C = depth;
    D_8013AE8C->data1E = index;
    D_8013AE8C->resource14 = allocateTaskResource();
    initMovement2D((Movement2D *)&D_800D9504[D_8013AE8C->resource14], x << 12, y << 12);
    D_8013AE8C->resource16 = allocateTaskResource();
    sequences = (TimedSequence *)&D_800D9504[D_8013AE8C->resource16];
    for (i = 0; i < 2; i++) {
        type = D_8012D460[selection[i].selected];
        if (type == 3 || type == 4 || type == 8) {
            initTimedSequence(&sequences[i], D_8012F1AC, 0, 8);
        } else {
            initTimedSequence(&sequences[i], D_8012F18C, 0, 4);
        }
    }
    if (index % 10 == 0) {
        ValuePanelEntry *table;
        if (D_8013AE88->resource14 == 0xFFFF) { while (1) {} }
        table = (ValuePanelEntry *)&D_800D9504[D_8013AE88->resource14];
        entry = table + index / 10;
    } else {
        ValuePanelEntry *table;
        if (D_8013AE88->resource16 == 0xFFFF) { while (1) {} }
        table = (ValuePanelEntry *)&D_800D9504[D_8013AE88->resource16];
        entry = table + index / 10;
    }
    entry->flags = 0;
    entry->active = 0;
    entry->task = D_8013AE8C;
}

/* DECOMP_ATTEMPT: inline 463/511 */
#ifdef NON_MATCHING
extern u8 D_8013B3A8 __attribute__((section(".data")));
extern u8 D_8013B3A9 __attribute__((section(".data")));
extern u8 D_8013B3AA __attribute__((section(".data")));

void func_80078140(void) {
    Movement2D *movement;
    TimedSequence *sequences;
    ValuePanelEntry *entry;
    SelectionEntry *selection;
    s32 sprites[2];
    s32 yOffsets[2];
    s32 depths[2];
    s32 shiftLastPortrait;
    s32 i;
    s32 xOffset;
    s32 frame;
    u8 type;

    if (D_8013AE88->resource14 == 0xFFFF) { while (1) {} }
    movement = (Movement2D *)&D_800D9504[D_8013AE88->resource14];
    if (D_8013AE88->resource16 == 0xFFFF) { while (1) {} }
    sequences = (TimedSequence *)&D_800D9504[D_8013AE88->resource16];

    if ((u16)(D_8013AE88->data1E % 10U) == 0) {
        ValuePanelEntry *entries;
        if (D_8013AE98->resource14 == 0xFFFF) { while (1) {} }
        entries = (ValuePanelEntry *)&D_800D9504[D_8013AE98->resource14];
        entry = &entries[D_8013AE88->data1E / 10U];
    } else {
        ValuePanelEntry *entries;
        if (D_8013AE98->resource16 == 0xFFFF) { while (1) {} }
        entries = (ValuePanelEntry *)&D_800D9504[D_8013AE98->resource16];
        entry = &entries[D_8013AE88->data1E / 10U];
    }

    if (D_8013AE98->resource14 == 0xFFFF) { while (1) {} }
    selection = (SelectionEntry *)&D_800D9504[D_8013AE98->resource14];

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
            s16 y;
            if (selection[i].unk00 == 1) {
                setSpriteColor(D_8013B3A8, D_8013B3A9, D_8013B3AA);
            } else if (selection[i].unk00 == 2) {
                setSpriteColor(gTaskWidgetRedAbsolute, gTaskWidgetGreenAbsolute, gTaskWidgetBlueAbsolute);
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
            y = selected / 2 * 32 + ((u16)yOffsets[i] + 36);
            y += movement->y / 4096;
            drawSpriteByIndexWithBlend(x, y,
                          sprites[i], (s16)i, -1, &D_800EBFF0[D_8013AF78], depths[i]);
            if (selection[i].selected != selection[i].previous) {
                type = D_8012D460[selection[i].selected];
                if (type == 3 || type == 4 || type == 8) {
                    initTimedSequence(&sequences[i], D_8012F1AC, 0, 8);
                } else {
                    initTimedSequence(&sequences[i], D_8012F18C, 0, 4);
                }
            }
        }
    }

    setSpriteColor(128, 128, 128);
    drawSpriteByIndexWithBlend(movement->x / 4096, movement->y / 4096, 92, 13, -1,
                  &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);

    for (i = 0; i < 10; i++) {
        s16 x;
        s16 y;
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
                frame = *(const s16 *)&sequences[0].entries[sequences[0].index].unk00;
            } else {
                frame = *(const s16 *)&sequences[1].entries[sequences[1].index].unk00;
            }
            x = xOffset + 4;
            x += i % 2 * 32;
            x += movement->x / 4096;
            y = movement->y / 4096 + 40;
            y = i / 2 * 32 + y;
            drawSpriteByIndexWithBlend(x, y,
                          D_8012D460[i] * 3 + (frame + 112), 0, -1,
                          &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
        } else {
            setSpriteColor(64, 64, 64);
            x = xOffset + 4;
            x += i % 2 * 32;
            x += movement->x / 4096;
            y = movement->y / 4096 + 40;
            y = i / 2 * 32 + y;
            drawSpriteByIndexWithBlend(x, y,
                          D_8012D460[i] * 3 + 112, 0, -1,
                          &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
        }
    }

    updateTimedSequence(&sequences[0]);
    updateTimedSequence(&sequences[1]);
}
#else
INCLUDE_ASM("asm/nonmatchings/menu/task_widgets", func_80078140);
#endif

void createPlayerPanelTask(s16 player, s16 x, s16 y, s32 depth, s32 index) {
    ValuePanelEntry *entry;

    createChildTaskWithReserve(updatePlayerPanelTask);
    D_8013AE8C->data1A = player;
    D_8013AE8C->data1C = depth;
    D_8013AE8C->data1E = index;
    D_8013AE8C->resource14 = allocateTaskResource();
    initMovement2D((Movement2D *)&D_800D9504[D_8013AE8C->resource14], x << 12, y << 12);
    D_8013AE8C->resource16 = allocateTaskResource();
    if (index % 10 == 0) {
        ValuePanelEntry *table;
        if (D_8013AE88->resource14 == 0xFFFF) {
            while (1) {}
        }
        table = (ValuePanelEntry *)&D_800D9504[D_8013AE88->resource14];
        entry = table + index / 10;
    } else {
        ValuePanelEntry *table;
        if (D_8013AE88->resource16 == 0xFFFF) {
            while (1) {}
        }
        table = (ValuePanelEntry *)&D_800D9504[D_8013AE88->resource16];
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
    position = (Movement2D *)&D_800D9504[D_8013AE88->resource14];
    if (D_8013AE88->resource16 == 0xFFFF) { while (1) {} }
    animation = (PlayerPanelAnimation *)&D_800D9504[D_8013AE88->resource16];
    if ((u16)(D_8013AE88->data1E % 10U) == 0) {
        if (D_8013AE98->resource14 == 0xFFFF) { while (1) {} }
        entries = (ValuePanelEntry *)&D_800D9504[D_8013AE98->resource14];
        panel = &entries[D_8013AE88->data1E / 10U];
    } else {
        if (D_8013AE98->resource16 == 0xFFFF) { while (1) {} }
        entries = (ValuePanelEntry *)&D_800D9504[D_8013AE98->resource16];
        panel = &entries[D_8013AE88->data1E / 10U];
    }
    if (D_8013AE98->resource14 == 0xFFFF) { while (1) {} }
    selections = (SelectionEntry *)&D_800D9504[D_8013AE98->resource14];
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

    if (D_8013AEE8[playerIndex] & 0x8000) {
        s16 current = *(volatile s16 *)&entry->selected;
        entry->selected = (current & 1) ? current - 1 : current + 1;
        selection = entry->selected - 8;
        if ((u16)selection < 2 && !(gMergedSaveRecord.flags & (1 << selection))) {
            entry->selected = previous;
        }
    }
    if (D_8013AEE8[playerIndex] & 0x2000) {
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
    if (D_8013AEE8[playerIndex] & 0x1000) {
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
    if (D_8013AEE8[playerIndex] & 0x4000) {
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
    if (D_8013AEE8[playerIndex] & 0xB0) {
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
    if (D_8013AEE8[playerIndex] & 0x40) {
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
    D_8013AE8C->resource14 = allocateTaskResource();
    resource = (SelectionPanelResource *)&D_800D9504[D_8013AE8C->resource14];
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
        table = (ValuePanelEntry *)&D_800D9504[D_8013AE88->resource14];
        entry = table + index / 10;
    } else {
        ValuePanelEntry *table;
        if (D_8013AE88->resource16 == 0xFFFF) { while (1) {} }
        table = (ValuePanelEntry *)&D_800D9504[D_8013AE88->resource16];
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
    TaskResourceRecord *slot;
    GameTask *parent;
    s16 player;
    u8 shade;

    if (D_8013AE88->resource14 == 0xFFFF) { while (1) {} }
    slot = &D_800D9504[D_8013AE88->resource14];
    if (D_8013AE98->resource14 == 0xFFFF) { while (1) {} }
    parent = D_8013AE98;
    position = (Movement2D *)&D_800D9504[parent->resource14];
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
    D_8013AE8C->resource14 = allocateTaskResource();
    resource = (ValuePanelResource *)&D_800D9504[D_8013AE8C->resource14];
    resource->values = values;
    resource->x = x << 12;
    resource->y = y << 12;
    D_8013AE8C->resource16 = allocateTaskResource();
    if (index % 10 == 0) {
        ValuePanelEntry *table;
        if (D_8013AE88->resource14 == 0xFFFF) {
            while (1) {}
        }
        table = (ValuePanelEntry *)&D_800D9504[D_8013AE88->resource14];
        entry = table + index / 10;
    } else {
        ValuePanelEntry *table;
        if (D_8013AE88->resource16 == 0xFFFF) {
            while (1) {}
        }
        table = (ValuePanelEntry *)&D_800D9504[D_8013AE88->resource16];
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
    resource = (ValuePanelResource *)&D_800D9504[D_8013AE88->resource14];
    if (D_8013AE88->resource16 == 0xFFFF) {
        while (1) {}
    }
    result = (ValuePanelEntry *)&D_800D9504[D_8013AE88->resource16];
    if ((u16)(D_8013AE88->data1E % 10U) == 0) {
        if (D_8013AE98->resource14 == 0xFFFF) {
            while (1) {}
        }
        entries = (ValuePanelEntry *)&D_800D9504[D_8013AE98->resource14];
        entry = &entries[D_8013AE88->data1E / 10U];
    } else {
        if (D_8013AE98->resource16 == 0xFFFF) {
            while (1) {}
        }
        entries = (ValuePanelEntry *)&D_800D9504[D_8013AE98->resource16];
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
    func_8006EF30((s16)(resource->x / 4096 + 48), (s16)(resource->y / 4096 + 16),
                   14, gValuePanelLabel, 15, -1, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C - 2);
    drawDefaultTexturedPanel((s16)(resource->x / 4096), (s16)(resource->y / 4096),
                             240, 64, 5, -1, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C - 2);
    drawScreenDarkeningOverlay(64, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
}

GameTask *createShrinkingSpriteTask(s16 motionEnabled, s32 spriteIndex) {
    SpriteZoomResource *resource;
    TaskResourceRecord *resources;
    createChildTaskWithReserve(updateShrinkingSpriteTask);
    D_8013AE8C->data18 = 1;
    D_8013AE8C->data1A = 1;
    D_8013AE8C->data1C = spriteIndex;
    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resources = D_800D9504;
    D_8013AE8C->resource14 = allocateTaskResource();
    resource = (SpriteZoomResource *)&resources[D_8013AE8C->resource14];
    resource->scale.value = 0x8000;
    resource->step = 0x400;
    D_8013AE8C->resource16 = allocateTaskResource();
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
    zoom = (SpriteZoomResource *)&D_800D9504[D_8013AE88->resource14];
    if (D_8013AE88->resource16 == 0xFFFF) {
        while (1) {}
    }
    motion = (SpriteMotionResource *)&D_800D9504[D_8013AE88->resource16];
    if (D_8013AE98->resource14 == 0xFFFF) {
        while (1) {}
    }
    position = (Movement2D *)&D_800D9504[D_8013AE98->resource14];
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

    if (D_8013AEE8[recordIndex] & 8) {
        cycleSaveRecordUnlockFlags(recordIndex, maskIndex);
    } else if (D_8013AEE8[recordIndex] & 2) {
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

    if (D_8013AEE8[recordIndex] & 4) {
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
    if (D_8013AEE8[recordIndex] & 1) {
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
    func_8006EF30(-112, -24, 14, gTextPanelMessage, 8, -1,
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
    D_8013AE88->resource14 = allocateTaskResource();
    if (D_8013AE88->resource16 != 0xFFFF) {
        while (1) {}
    }
    D_8013AE88->resource16 = allocateTaskResource();
    setCurrentTaskCallback(updateMainMenuSelectionTask);
    D_8013AE88->unk10 |= 2;
}
