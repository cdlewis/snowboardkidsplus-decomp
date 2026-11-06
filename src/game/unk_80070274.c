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

/* Storage remains in the shared data object. */
typedef struct TaskValueState {
    GameTask *task;
    u16 unk04;
    u16 unk06;
} TaskValueState;

/* Storage remains in the shared data object. */
extern TaskValueState D_8012D4A4[3];
extern GameTask *D_8013B378;
extern GameTask *D_8013B37C;
extern GameTask *D_8013BDA8[4];

s8 gTaskImageSourceRows[12] = {8, 1, 2, 3, 4, 5, 6, 7, 9, 0, 0, 0};

s16 func_80070794(s32 id);
void func_80072340(s32 x, s32 y, s32 width, s32 height, s32 r, s32 g, s32 b,
                   GsOT *ot, u16 depth);
void func_8007197C(void);
void updateCountdownTask(void);
void func_80076034(void);
void func_8007724C(void);

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

INCLUDE_ASM("asm/nonmatchings/game/unk_80070274", func_80070E34);

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
        func_80072340(-160, -120, 320, 240, 0, 0, 0,
                     &D_800EBFF0[D_8013AF78], 0xFFE);
    }
}

INCLUDE_ASM("asm/nonmatchings/game/unk_80070274", func_80071700);

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

INCLUDE_ASM("asm/nonmatchings/game/unk_80070274", func_80072340);

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

INCLUDE_ASM("asm/nonmatchings/game/unk_80070274", func_800775C8);

INCLUDE_ASM("asm/nonmatchings/game/unk_80070274", func_800776B0);

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

INCLUDE_ASM("asm/nonmatchings/game/unk_80070274", func_8007B034);

INCLUDE_ASM("asm/nonmatchings/game/unk_80070274", func_8007B10C);

INCLUDE_ASM("asm/nonmatchings/game/unk_80070274", func_8007B48C);

INCLUDE_ASM("asm/nonmatchings/game/unk_80070274", func_8007B890);

INCLUDE_ASM("asm/nonmatchings/game/unk_80070274", func_8007BBAC);

INCLUDE_ASM("asm/nonmatchings/game/unk_80070274", func_8007BD04);

INCLUDE_ASM("asm/nonmatchings/game/unk_80070274", func_8007BDE0);
