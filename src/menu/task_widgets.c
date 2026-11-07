#define TASK_ABSOLUTE_ADDRESSING
#define MUSIC_STATE_ABSOLUTE_ADDRESSING
#include "common.h"
#define SAVE_OVERRIDE_ABSOLUTE_ADDRESSING
#include "game/save/save_data.h"
#include "game/engine/game_options.h"
#include "game/engine/byte_search.h"
#include "game/io/controller.h"
#include "game/graphics/graphics.h"
#include "game/engine/game_task_scheduler.h"
#include "game/engine/linear_random.h"
#include "game/engine/offset_archive.h"
#include "game/audio/music.h"
#include "game/menu/movement.h"
#define TASK_ABSOLUTE_ADDRESSING
#include "game/engine/task.h"
#include "game/menu/selected_sprite.h"
#include "psx/libsnd.h"
#include "psx/stdlib.h"


typedef struct SpacedTaskResource {
    s32 x, y;
    u8 unk08[0x38];
    TaskCallback callback;
    u8 unk44[0xC];
    s32 value;
    s16 unk54, spacingX, spacingY, count;
} SpacedTaskResource;

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

typedef struct TimedSequenceEntry {
    s32 unk00;
    s32 duration;
} TimedSequenceEntry;

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
    s32 scale;
    u8 unk04[0x10];
    s32 step;
    u8 unk18[0xE8];
} SpriteZoomResource;

typedef struct SelectionPanelRecord {
    s16 unk00, selection, previousSelection;
} SelectionPanelRecord;
typedef struct SelectionPanelResource {
    u8 unk00[8];
    s8 unk08;
    u8 unk09[0xB];
    SelectionPanelRecord *record;
} SelectionPanelResource;

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

typedef struct ValuePanelEntry {
    GameTask *task;
    s16 flags;
    s16 active;
} ValuePanelEntry;

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

TimedSequenceEntry gSecondaryTaskSequenceEntries[6]
    __attribute__((section(".data.secondary_task_sequence"))) = {
    {0x000C0172, 6}, {0x000C0173, 6}, {0x000C0174, 6},
    {0x000C0175, 6}, {0x000C0176, 6}, {0x000C0177, 6}
};

u16 gTextPanelMessage[] __attribute__((section(".data.text_panel_message"))) = {
    0xC00F, 0x0049, 0x0040, 0x0069, 0x004A, 0x0052,
    0x0158, 0x00BF, 0x0096, 0x00B9, 0x00C0, 0x00CE,
    0x009A, 0x00CB, 0x009D, 0x0094, 0x0033, 0x0000
};

/* Storage remains in the shared data object. */
extern u8 D_8012D460[12];
extern TaskValueState D_8012D4A4[3];
extern TimedSequenceEntry D_8012F18C[], D_8012F1AC[];
extern GameTask *D_8013B378;
extern GameTask *D_8013B37C;
extern GameTask *D_8013BDA8[4];

s8 gTaskImageSourceRows[12] = {8, 1, 2, 3, 4, 5, 6, 7, 9, 0, 0, 0};

SPRT_16 gAnimatedSprite16Template __attribute__((section(".data.animated_sprite16"))) = {
    0x03000000, 0x80, 0x80, 0x80, SPRT_16_CODE,
    0, 0, 0x70, 0x50, 0x3EB2
};
DR_TPAGE gAnimatedSprite16DrawPage __attribute__((section(".sdata.animated_sprite16_page"))) = {
    0x01000000, {0xE1000000 | getTPage(0, 0, 832, 256)}
};

s16 func_80070794(s32 id);
void func_80072A88(s16 index, s16 x, s16 y, TaskCallback callback, s32 value, s32 registrationIndex);
void func_80072C54(void);

void addTilePacket(s32 x, s32 y, s32 width, s32 height, s32 r, s32 g, s32 b,
                   GsOT *ot, u16 depth);
void updateScrollingBackgroundTask(void);
void drawTaskImagesOnBlackBackground(void);
void func_8007195C(void);
void func_80070448(void);
void func_8007B890(s16 recordIndex, s16 maskIndex);
void func_8007197C(void);
void func_80071A80(void);
void initTimedSequence(TimedSequence *sequence, const TimedSequenceEntry *entries, s16 index, s16 count);
void updateCountdownTask(void);
void func_8007649C(void);
void func_80076E18(void);
void updateTimedSpriteB5Task(void);
void updateValueTransitionTask(void);
void func_80077770(void);
void func_80078140(void);
void func_80078AFC(void);
void func_80079FE8(void);
void GsSortBg(GameBackground *background, GsOT *ot, u16 depth);
void func_8007BE9C(void);
void func_8007B10C(void);
void func_8007AC0C(void);
void updateAnimatedSpriteTask(void);
void func_80074448(void);
void func_80074B60(void);

void configureTaskStateTransition(s32 mode, s32 id, u16 duration) {
    TaskValueState *state;
    s16 index;
    index = func_80070794(id);
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
        func_80014930(&gGameTasks[4], func_80070448);
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

INCLUDE_ASM("asm/nonmatchings/menu/task_widgets", func_80070448);

INCLUDE_ASM("asm/nonmatchings/menu/task_widgets", func_80070794);

s32 hasTaskStateValue6(s32 id) {
    return D_8012D4A4[func_80070794(id)].unk06 != 0;
}

INCLUDE_ASM("asm/nonmatchings/menu/task_widgets", func_800707F0);

INCLUDE_ASM("asm/nonmatchings/menu/task_widgets", func_80070830);

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
                func_80014930(&gGameTasks[3], updateColorCycleTask);
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
        func_800149A8();
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

INCLUDE_ASM("asm/nonmatchings/menu/task_widgets", func_80070B64);

s32 isTaskResourceTransitionComplete(s16 index) {
    GameTask *task;
    task = *(index + D_8013BDA8);
    if (task != NULL && task->data1E == 1) {
        return 1;
    }
    return 0;
}

INCLUDE_ASM("asm/nonmatchings/menu/task_widgets", func_80070C0C);

INCLUDE_ASM("asm/nonmatchings/menu/task_widgets", func_80070C5C);

INCLUDE_ASM("asm/nonmatchings/menu/task_widgets", func_80070D48);

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

INCLUDE_ASM("asm/nonmatchings/menu/task_widgets", func_8007102C);

INCLUDE_ASM("asm/nonmatchings/menu/task_widgets", func_80071300);

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
        func_80014930(&gGameTasks[3], drawBlackScreenTask);
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
        func_80014930(&gGameTasks[3], updateScrollingBackgroundTask);
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
        func_80014930(&gGameTasks[3], drawTaskImagesOnBlackBackground);
        D_8013B378 = D_8013AE8C;
        D_8013B378->data18 = mode;
        D_8013B378->data1A = value1;
        D_8013B378->data1C = 0;
        copyIndexedTaskImagesToVram(value1);
        func_8007195C();
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
        func_800149A8();
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
        func_800149A8();
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
        func_800149A8();
    } else {
        setSpriteColor(128, 128, 128);
        func_8006C998(-76, -11, 296, 0, -1, &D_800EBFF0[D_8013AF78], 1);
        func_8006C998(-84, -16, 295, 1, -1, &D_800EBFF0[D_8013AF78], 1);
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
    func_8007197C();
}

INCLUDE_ASM("asm/nonmatchings/menu/task_widgets", func_8007195C);

INCLUDE_ASM("asm/nonmatchings/menu/task_widgets", func_8007197C);

void clearSecondaryTaskReference(void) {
    D_8013B37C = NULL;
}

void setSecondaryDisplayTaskMode(s16 mode) {
    switch (mode) {
    case 1: case 2: case 3:
        if (D_8013B37C != NULL) {
            D_8013B37C->data18 = 0;
        }
        func_80014930(&gGameTasks[3], func_80071A80);
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

INCLUDE_ASM("asm/nonmatchings/menu/task_widgets", func_80071A80);

INCLUDE_ASM("asm/nonmatchings/menu/task_widgets", func_80071E20);

void markSecondaryTaskAndClearReference(void) {
    if (D_8013B37C != NULL) {
        D_8013B37C->data18 = 99;
        D_8013B37C = NULL;
    }
}

INCLUDE_ASM("asm/nonmatchings/menu/task_widgets", func_80071E78);

INCLUDE_ASM("asm/nonmatchings/menu/task_widgets", func_80071EF8);

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

INCLUDE_ASM("asm/nonmatchings/menu/task_widgets", func_8007240C);

void setMusicSequenceEnabled(s16 enabled) {
    if (enabled == 0) {
        if (D_8013B106 == 1) {
            fadeOutMusicSequence(0x200);
            D_8013B106 = 0;
        }
    } else if (D_8013B106 == 0) {
        func_80017E7C(0);
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
        func_80014874(initializeSpacedChildTasks);
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
        func_80072A88(i, state->x / 4096 + state->spacingX * i,
                     state->y / 4096 + state->spacingY * i,
                     state->callback, state->value, i * 10 + 1);
    }
    entry->flags = 0;
    setCurrentTaskCallback(func_80072C54);
}

INCLUDE_ASM("asm/nonmatchings/menu/task_widgets", func_80072A88);

INCLUDE_ASM("asm/nonmatchings/menu/task_widgets", func_80072C54);

INCLUDE_ASM("asm/nonmatchings/menu/task_widgets", func_800736D0);

INCLUDE_ASM("asm/nonmatchings/menu/task_widgets", func_80073708);

INCLUDE_ASM("asm/nonmatchings/menu/task_widgets", func_8007373C);

INCLUDE_ASM("asm/nonmatchings/menu/task_widgets", func_80073760);

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
        func_8006C998(position->x / 4096, position->y / 4096, state->sprite, 0, -1,
                     &D_800EBFF0[D_8013AF78], state->depth - 1);
    }
}

INCLUDE_ASM("asm/nonmatchings/menu/task_widgets", func_80073F84);



void createRegisteredTextPanelTask(s16 x, s16 y, const u16 **texts, const u8 *mapping, s32 depth, s32 index) {
    RegisteredTextResource *resource;
    RegisteredTaskEntry *entry;
    func_80014874(func_80074448);
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

INCLUDE_ASM("asm/nonmatchings/menu/task_widgets", func_80074448);

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
    func_80014874(func_80074B60);
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

INCLUDE_ASM("asm/nonmatchings/menu/task_widgets", func_80074B60);

INCLUDE_ASM("asm/nonmatchings/menu/task_widgets", func_80074E2C);

INCLUDE_ASM("asm/nonmatchings/menu/task_widgets", func_800750E0);

INCLUDE_ASM("asm/nonmatchings/menu/task_widgets", func_800757E0);

GameTask *createValueTransitionTask(u32 *value, u32 target, u32 step) {
    ValueTransitionResource *resource;
    func_80014874(updateValueTransitionTask);
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
        func_800149A8();
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

    func_80014874(updateCountdownTask);
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
        func_800149A8();
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
    func_80014874(func_8007649C);
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

INCLUDE_ASM("asm/nonmatchings/menu/task_widgets", func_8007649C);

void createInputTimedSpriteTask(s32 x, s32 y, const TimedSequenceEntry *entries, s32 count,
    s32 controllers, u32 acceptMask, u32 cancelMask, s32 depth, s32 index) {
    InputTimedSpriteResource *resource;
    ValuePanelEntry *entry;

    func_80014874(func_80076E18);
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

INCLUDE_ASM("asm/nonmatchings/menu/task_widgets", func_80076E18);

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
    func_80014874(updateTimedSpriteB5Task);
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
        func_800149A8();
    }
    setSpriteColor(128, 128, 128);
    func_8006C998(movement->x / 4096, movement->y / 4096, 0xB5, 0, -1,
                 &D_800EBFF0[D_8013AF78], 3);
    lifetime->unk04--;
}

GameTask *createAnimatedSpriteTask(s16 x, s16 y, s16 unk02, s16 repeats, u16 depth) {
    GameTask *task;
    AnimatedSpriteResource *resource;
    func_80014874(updateAnimatedSpriteTask);
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
        func_800149A8();
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

INCLUDE_ASM("asm/nonmatchings/menu/task_widgets", func_80077770);

INCLUDE_ASM("asm/nonmatchings/menu/task_widgets", func_80077E58);

void createDualSelectionSequenceTask(s16 x, s16 y, s32 depth, s32 index) {
    SelectionEntry *selection;
    TimedSequence *sequences;
    ValuePanelEntry *entry;
    s16 i;
    u8 type;
    if (D_8013AE88->resource14 == 0xFFFF) { while (1) {} }
    selection = (SelectionEntry *)&D_800D9504[D_8013AE88->resource14];
    func_80014874(func_80078140);
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

INCLUDE_ASM("asm/nonmatchings/menu/task_widgets", func_80078140);

void createPlayerPanelTask(s16 player, s16 x, s16 y, s32 depth, s32 index) {
    ValuePanelEntry *entry;

    func_80014874(func_80078AFC);
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

INCLUDE_ASM("asm/nonmatchings/menu/task_widgets", func_80078AFC);

INCLUDE_ASM("asm/nonmatchings/menu/task_widgets", func_800797C4);

void createRegisteredSelectionPanelTask(s16 playerIndex, SelectionPanelRecord *record, s32 depth, s32 index) {
    SelectionPanelResource *resource;
    ValuePanelEntry *entry;
    s32 player;
    func_80014874(func_80079FE8);
    D_8013AE8C->data1A = playerIndex;
    D_8013AE8C->data1C = depth;
    D_8013AE8C->data1E = index;
    D_8013AE8C->resource14 = allocateTaskResource();
    resource = (SelectionPanelResource *)&D_800D9504[D_8013AE8C->resource14];
    resource->unk08 = 0;
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

INCLUDE_ASM("asm/nonmatchings/menu/task_widgets", func_80079FE8);

void createValuePanelTask(s16 x, s16 y, s16 *values, s32 depth, s32 index) {
    ValuePanelResource *resource;
    ValuePanelEntry *entry;

    func_80014874(func_8007AC0C);
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

INCLUDE_ASM("asm/nonmatchings/menu/task_widgets", func_8007AC0C);

GameTask *createShrinkingSpriteTask(s16 motionEnabled, u16 spriteIndex) {
    SpriteZoomResource *resource;
    TaskResourceRecord *resources;
    func_80014874(func_8007B10C);
    D_8013AE8C->data18 = 1;
    D_8013AE8C->data1A = 1;
    D_8013AE8C->data1C = spriteIndex;
    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resources = D_800D9504;
    D_8013AE8C->resource14 = allocateTaskResource();
    resource = (SpriteZoomResource *)&resources[D_8013AE8C->resource14];
    resource->scale = 0x8000;
    resource->step = 0x400;
    D_8013AE8C->resource16 = allocateTaskResource();
    (*(SpriteMotionResource *)&resources[D_8013AE8C->resource16])[0] = motionEnabled;
    return D_8013AE8C;
}

INCLUDE_ASM("asm/nonmatchings/menu/task_widgets", func_8007B10C);

INCLUDE_ASM("asm/nonmatchings/menu/task_widgets", func_8007B48C);

INCLUDE_ASM("asm/nonmatchings/menu/task_widgets", func_8007B890);

void applySaveRecordUnlockOverrides(s16 index) {
    s16 i;
    if (D_8013B11A & 1) {
        updatePairedStateFlag(0);
        updatePairedStateFlag(1);
    }
    if (D_8013B11A & 2) {
        for (i = 0; i < 10; i++) {
            setSaveRecordMaskTier(index, i, 4);
            func_8007B890(index, i);
            func_8007B890(index, i);
            func_8007B890(index, i);
            func_8007B890(index, i);
            func_8007B890(index, i);
            func_8007B890(index, i);
        }
        gUnlockNotificationFlags = 0x7FF;
    }
    if (D_8013B11A & 4) {
        SaveRecord *records = (SaveRecord *)&D_8013080C;
        records[index].flags_3AC |= 0x1FFFC;
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
    setCurrentTaskCallback(func_8007BE9C);
    D_8013AE88->unk10 |= 2;
}
