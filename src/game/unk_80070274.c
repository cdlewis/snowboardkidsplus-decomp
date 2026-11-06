#define TASK_ABSOLUTE_ADDRESSING
#define MUSIC_STATE_ABSOLUTE_ADDRESSING
#include "common.h"
#include "game/byte_search.h"
#include "game/graphics.h"
#include "game/game_task_scheduler.h"
#include "game/linear_random.h"
#include "game/offset_archive.h"
#include "game/music.h"
#include "game/movement.h"
#define TASK_ABSOLUTE_ADDRESSING
#include "game/task.h"
#include "psx/libsnd.h"


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

typedef struct SpriteZoomResource {
    s32 scale;
    u8 unk04[0x10];
    s32 step;
    u8 unk18[0xE8];
} SpriteZoomResource;

typedef s16 SpriteMotionResource[128];

/* Storage remains in the shared data object. */
typedef struct TaskValueState {
    GameTask *task;
    u16 unk04;
    u16 unk06;
} TaskValueState;

u16 gTextPanelMessage[] __attribute__((section(".data.text_panel_message"))) = {
    0xC00F, 0x0049, 0x0040, 0x0069, 0x004A, 0x0052,
    0x0158, 0x00BF, 0x0096, 0x00B9, 0x00C0, 0x00CE,
    0x009A, 0x00CB, 0x009D, 0x0094, 0x0033, 0x0000
};

/* Storage remains in the shared data object. */
extern TaskValueState D_8012D4A4[3];
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
void addTilePacket(s32 x, s32 y, s32 width, s32 height, s32 r, s32 g, s32 b,
                   GsOT *ot, u16 depth);
void func_80071490(s32 mode, s32 value1, s32 value2);
void func_8006EF30(s32 x, s32 y, s16 columns, const u16 *text,
                   s32 palette, s16 blendMode, GsOT *ot, u16 depth);
void func_8007197C(void);
void updateCountdownTask(void);
void func_80076034(void);
void func_8007724C(void);
void func_80077770(void);
void GsSortBg(GameBackground *background, GsOT *ot, u16 depth);
void func_8007BE9C(void);
void func_8007B10C(void);

INCLUDE_ASM("asm/nonmatchings/game/unk_80070274", func_80070274);

INCLUDE_ASM("asm/nonmatchings/game/unk_80070274", func_80070448);

INCLUDE_ASM("asm/nonmatchings/game/unk_80070274", func_80070794);

s32 hasTaskStateValue6(s32 id) {
    return D_8012D4A4[func_80070794(id)].unk06 != 0;
}

INCLUDE_ASM("asm/nonmatchings/game/unk_80070274", func_800707F0);

INCLUDE_ASM("asm/nonmatchings/game/unk_80070274", func_80070830);

INCLUDE_ASM("asm/nonmatchings/game/unk_80070274", func_80070870);

INCLUDE_ASM("asm/nonmatchings/game/unk_80070274", func_80070A2C);

INCLUDE_ASM("asm/nonmatchings/game/unk_80070274", func_80070B64);

s32 isTaskResourceTransitionComplete(s16 index) {
    GameTask *task;
    task = *(index + D_8013BDA8);
    if (task != NULL && task->data1E == 1) {
        return 1;
    }
    return 0;
}

INCLUDE_ASM("asm/nonmatchings/game/unk_80070274", func_80070C0C);

INCLUDE_ASM("asm/nonmatchings/game/unk_80070274", func_80070C5C);

INCLUDE_ASM("asm/nonmatchings/game/unk_80070274", func_80070D48);

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

INCLUDE_ASM("asm/nonmatchings/game/unk_80070274", func_80070F10);

INCLUDE_ASM("asm/nonmatchings/game/unk_80070274", func_8007102C);

INCLUDE_ASM("asm/nonmatchings/game/unk_80070274", func_80071300);

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

INCLUDE_ASM("asm/nonmatchings/game/unk_80070274", func_80071490);

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

INCLUDE_ASM("asm/nonmatchings/game/unk_80070274", func_800717E8);

s32 hasTaskReference(void) {
    return D_8013B378 != NULL;
}

void resetTaskControlState(void) {
    func_80070274(0, 0, 30);
    func_80071490(0, 0, 0);
    func_8007197C();
}

INCLUDE_ASM("asm/nonmatchings/game/unk_80070274", func_8007195C);

INCLUDE_ASM("asm/nonmatchings/game/unk_80070274", func_8007197C);

void clearSecondaryTaskReference(void) {
    D_8013B37C = NULL;
}

INCLUDE_ASM("asm/nonmatchings/game/unk_80070274", func_800719A8);

INCLUDE_ASM("asm/nonmatchings/game/unk_80070274", func_80071A80);

INCLUDE_ASM("asm/nonmatchings/game/unk_80070274", func_80071E20);

void markSecondaryTaskAndClearReference(void) {
    if (D_8013B37C != NULL) {
        D_8013B37C->data18 = 99;
        D_8013B37C = NULL;
    }
}

INCLUDE_ASM("asm/nonmatchings/game/unk_80070274", func_80071E78);

INCLUDE_ASM("asm/nonmatchings/game/unk_80070274", func_80071EF8);

INCLUDE_ASM("asm/nonmatchings/game/unk_80070274", func_8007202C);

INCLUDE_ASM("asm/nonmatchings/game/unk_80070274", func_8007219C);

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

INCLUDE_ASM("asm/nonmatchings/game/unk_80070274", func_8007240C);

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

INCLUDE_ASM("asm/nonmatchings/game/unk_80070274", func_800725D0);

INCLUDE_ASM("asm/nonmatchings/game/unk_80070274", func_80072864);

INCLUDE_ASM("asm/nonmatchings/game/unk_80070274", func_80072A88);

INCLUDE_ASM("asm/nonmatchings/game/unk_80070274", func_80072C54);

INCLUDE_ASM("asm/nonmatchings/game/unk_80070274", func_800736D0);

INCLUDE_ASM("asm/nonmatchings/game/unk_80070274", func_80073708);

INCLUDE_ASM("asm/nonmatchings/game/unk_80070274", func_8007373C);

INCLUDE_ASM("asm/nonmatchings/game/unk_80070274", func_80073760);

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

INCLUDE_ASM("asm/nonmatchings/game/unk_80070274", func_80073810);

INCLUDE_ASM("asm/nonmatchings/game/unk_80070274", func_80073950);

INCLUDE_ASM("asm/nonmatchings/game/unk_80070274", func_80073B0C);

INCLUDE_ASM("asm/nonmatchings/game/unk_80070274", func_80073CB4);

INCLUDE_ASM("asm/nonmatchings/game/unk_80070274", func_80073E04);

INCLUDE_ASM("asm/nonmatchings/game/unk_80070274", func_80073F84);

INCLUDE_ASM("asm/nonmatchings/game/unk_80070274", func_80074270);

INCLUDE_ASM("asm/nonmatchings/game/unk_80070274", func_80074448);

void setTaskSecondaryResourceData(GameTask *task, const u16 *data) {
    u32 index = task->resource16;
    TaskResourceRecord *resource = &D_800D9504[index];
    resource->unk08 = data;
}

s16 hasTaskResource16Value(GameTask *task) {
    TaskResourceRecord *resource = task->resource16 + D_800D9504;
    return resource->unk08 != 0;
}

INCLUDE_ASM("asm/nonmatchings/game/unk_80070274", func_80074988);

INCLUDE_ASM("asm/nonmatchings/game/unk_80070274", func_80074B60);

INCLUDE_ASM("asm/nonmatchings/game/unk_80070274", func_80074E2C);

INCLUDE_ASM("asm/nonmatchings/game/unk_80070274", func_800750E0);

INCLUDE_ASM("asm/nonmatchings/game/unk_80070274", func_800757E0);

GameTask *createValueTransitionTask(u32 *value, u32 target, u32 step) {
    ValueTransitionResource *resource;
    func_80014874(func_80076034);
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

INCLUDE_ASM("asm/nonmatchings/game/unk_80070274", func_80076034);

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

INCLUDE_ASM("asm/nonmatchings/game/unk_80070274", func_80076258);

INCLUDE_ASM("asm/nonmatchings/game/unk_80070274", func_8007649C);

INCLUDE_ASM("asm/nonmatchings/game/unk_80070274", func_80076C20);

INCLUDE_ASM("asm/nonmatchings/game/unk_80070274", func_80076E18);

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
    func_80014874(func_8007724C);
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

INCLUDE_ASM("asm/nonmatchings/game/unk_80070274", func_8007724C);

INCLUDE_ASM("asm/nonmatchings/game/unk_80070274", func_80077390);

INCLUDE_ASM("asm/nonmatchings/game/unk_80070274", func_8007747C);

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

INCLUDE_ASM("asm/nonmatchings/game/unk_80070274", func_80077770);

INCLUDE_ASM("asm/nonmatchings/game/unk_80070274", func_80077E58);

INCLUDE_ASM("asm/nonmatchings/game/unk_80070274", func_80077EA4);

INCLUDE_ASM("asm/nonmatchings/game/unk_80070274", func_80078140);

INCLUDE_ASM("asm/nonmatchings/game/unk_80070274", func_8007893C);

INCLUDE_ASM("asm/nonmatchings/game/unk_80070274", func_80078AFC);

INCLUDE_ASM("asm/nonmatchings/game/unk_80070274", func_800797C4);

INCLUDE_ASM("asm/nonmatchings/game/unk_80070274", func_80079DB0);

INCLUDE_ASM("asm/nonmatchings/game/unk_80070274", func_80079FE8);

INCLUDE_ASM("asm/nonmatchings/game/unk_80070274", func_8007AA58);

INCLUDE_ASM("asm/nonmatchings/game/unk_80070274", func_8007AC0C);

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

INCLUDE_ASM("asm/nonmatchings/game/unk_80070274", func_8007B10C);

INCLUDE_ASM("asm/nonmatchings/game/unk_80070274", func_8007B48C);

INCLUDE_ASM("asm/nonmatchings/game/unk_80070274", func_8007B890);

INCLUDE_ASM("asm/nonmatchings/game/unk_80070274", func_8007BBAC);

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
