#include "common.h"
#include "main_tail_sound_slots.h"
#include "game/controller.h"
#include "game/global_flags.h"
#define SELECTED_RECORDS_ABSOLUTE_ADDRESSING
#include "game/selected_records.h"
#include "game/cd_queue.h"
#include "game/game_options.h"
#include "game/graphics.h"
#define MUSIC_ABSOLUTE_ADDRESSING
#include "game/music.h"
#include "psx/libsnd.h"
#define RACE_PLAYER_ABSOLUTE_ADDRESSING
#define TASK_ABSOLUTE_ADDRESSING
#define TASK_FLAGS_ABSOLUTE_ADDRESSING
#include "game/race_player.h"
#include "game/text_entries.h"
#include "game/task.h"
#include "psx/libgpu.h"
#include "psx/libgs.h"

typedef struct UnlockMenuState {
    s16 selection, previousSelection;
    s32 unk04[7];
    s32 unk20[7];
    s32 unk3C[7];
    s32 unk58[7];
    s16 unk74[4];
    u8 unk7C[0x10];
    s16 unk8C;
    u8 unk8E[0x72];
} UnlockMenuState;

typedef struct TextControlState {
    s16 unk00;
    s16 unk02;
    s32 unk04;
    s32 unk08;
    s32 unk0C;
    u8 unk10[8];
    s16 unk18[5];
} TextControlState;
typedef struct TextResourceState {
    TextEntry *currentEntry;
    u32 unk04[5];
    s16 unk18[5];
} TextResourceState;
typedef struct TextTaskResource {
    TextEntry *entry;
    u8 unk04[0x2C];
    GameTask *task;
    u8 unk34[0xCC];
} TextTaskResource;

typedef struct TextEntryListHeader {
    TextEntry *freeEntries;
    TextEntry *activeEntries;
} TextEntryListHeader;


extern u8 D_8013AF3A __attribute__((section(".data")));
extern s16 D_8013B10C __attribute__((section(".data")));
extern u8 D_8013B55E __attribute__((section(".data")));

extern s16 D_8013B560;
extern u8 D_8013B562;


void func_800187B8(u8 voice);

void func_800A3750(void);
void func_800A5AFC(void);
void func_800A6548(void);
void func_800A624C(void);

void func_8002FA58(s16 resource);
void func_800A39AC(void);
void func_800A42CC(void);
void func_800A499C(void);
void func_8001B858(void);
void func_800A1F50(void);
void func_800A4C28(void);
void resetUnlockMenuOffsets(UnlockMenuState *state);
void resetUnlockMenuAnimation(UnlockMenuState *state);
void func_8007BBAC(s16 index);
void func_800A3D9C(void);
void func_800A525C(void);
void func_800A6A8C(void);
void func_800A6E88(void);

typedef struct NumberAdjustmentState {
    s16 value;
    s16 countdown;
    s16 active;
    u8 unk06[0x1A];
    s32 unk20[3];
    s32 unk2C[3];
    u8 unk38[0xC8];
} NumberAdjustmentState;
typedef struct MusicOwnerState {
    u8 unk00[0x24];
    u32 tick;
    u8 unk28[0xC];
    GameTask *musicTask;
    u8 unk38[8];
    u32 completionTick;
} MusicOwnerState;
void func_800A53BC(void);
void func_800A624C(void);
void startUnlockResourceLoad(void);
void prepareUnlockSelectionResume(void);
extern u8 D_8013AF3A __attribute__((section(".data")));
INCLUDE_ASM("asm/nonmatchings/main_tail_ui", func_800A1F50);

INCLUDE_ASM("asm/nonmatchings/main_tail_ui", func_800A2210);

INCLUDE_ASM("asm/nonmatchings/main_tail_ui", func_800A2324);

void selectTaskBySelectedRecordMatch(void) {
    if (D_8013B560 != D_8013AFCE) {
        D_8013B562 = 0;
        setCurrentTaskCallback(func_8001B858);
    } else {
        D_8013B562 = 1;
        setCurrentTaskCallback(func_800A1F50);
    }
}

void startMode1TaskTransition(void) {
    setSecondaryDisplayTaskMode(1);
    func_80070274(1, 0, 30);
    setCurrentTaskCallback(initializeGameOptionViewTask);
}

INCLUDE_ASM("asm/nonmatchings/main_tail_ui", func_800A24EC);

INCLUDE_ASM("asm/nonmatchings/main_tail_ui", func_800A2510);

INCLUDE_ASM("asm/nonmatchings/main_tail_ui", func_800A25F8);

INCLUDE_ASM("asm/nonmatchings/main_tail_ui", func_800A29A4);

INCLUDE_ASM("asm/nonmatchings/main_tail_ui", func_800A2AB0);

INCLUDE_ASM("asm/nonmatchings/main_tail_ui", func_800A2C40);

INCLUDE_ASM("asm/nonmatchings/main_tail_ui", func_800A2C74);

INCLUDE_ASM("asm/nonmatchings/main_tail_ui", func_800A2D68);

INCLUDE_ASM("asm/nonmatchings/main_tail_ui", func_800A3028);

INCLUDE_ASM("asm/nonmatchings/main_tail_ui", func_800A31C0);

INCLUDE_ASM("asm/nonmatchings/main_tail_ui", func_800A3750);

void advanceTaskWhenFlagB55ESet(void) {
    if (D_8013B55E != 0) {
        D_8013AE88->data18 = 0;
        setCurrentTaskCallback(func_800A39AC);
    }
}

INCLUDE_ASM("asm/nonmatchings/main_tail_ui", func_800A39AC);

INCLUDE_ASM("asm/nonmatchings/main_tail_ui", func_800A3AA4);

void initializeUnlockMenuResourceTask(void) {
    UnlockMenuState *state;
    if (D_8013AE88->resource14 != 0xFFFF) {
        for (;;) {}
    }
    D_8013AE88->resource14 = allocateTaskResource();
    state = (UnlockMenuState *)&D_800D9504[D_8013AE88->resource14];
    state->selection = 0;
    state->previousSelection = 0;
    resetUnlockMenuAnimation(state);
    state->unk8C = 0;
    func_80070274(1, 0, 30);
    func_8007BBAC(0);
    setCurrentTaskCallback(func_800A3D9C);
}

void resetUnlockMenuAnimation(UnlockMenuState *state) {
    s16 i;
    state->unk04[0] = state->unk20[0] = 0;
    for (i = 1; i < 7; i++) {
        state->unk04[i] = state->unk3C[i] = state->unk04[0];
        state->unk20[i] = state->unk58[i] = state->unk20[0];
    }
    state->unk3C[0] = state->unk04[0];
    state->unk58[0] = state->unk20[0];
    state->unk74[0] = 0;
    state->unk74[1] = 1;
    state->unk74[2] = 2;
    state->unk74[3] = 3;
}

void runTaskResource14Update(void) {
    if (D_8013AE88->resource14 == 0xFFFF) {
        for (;;) {
        }
    }
    func_800A42CC();
}

INCLUDE_ASM("asm/nonmatchings/main_tail_ui", func_800A3D9C);

INCLUDE_ASM("asm/nonmatchings/main_tail_ui", func_800A42CC);

INCLUDE_ASM("asm/nonmatchings/main_tail_ui", func_800A46E0);

void initializeUnlockMenuTask(void) {
    UnlockMenuState *state;
    if (D_8013AE88->resource14 == 0xFFFF) {
        for (;;) {}
    }
    state = (UnlockMenuState *)&D_800D9504[D_8013AE88->resource14];
    state->selection = 0;
    state->previousSelection = 0;
    resetUnlockMenuOffsets(state);
    func_800A4C28();
    setCurrentTaskCallback(func_800A499C);
}

void resetUnlockMenuOffsets(UnlockMenuState *state) {
    s16 i;
    for (i = 0; i < 6; i++) {
        i[state->unk3C] = 0;
        i[state->unk04] = 0;
        i[state->unk58] = 0;
        i[state->unk20] = 0;
    }
}

INCLUDE_ASM("asm/nonmatchings/main_tail_ui", func_800A499C);

INCLUDE_ASM("asm/nonmatchings/main_tail_ui", func_800A4C28);

s32 hasUnlockNotificationFlag(s16 index) {
    if (index == 11) {
        index = 10;
    }
    return (s16)getUnlockNotificationFlag(index) != 0;
}

void startUnlockActionAfterCountdown(void) {
    if (--D_8013AE88->data18 == 0) {
        if (D_8013AF3A < 11) {
            func_80014874(func_800A624C);
        } else {
            func_80014874(startUnlockResourceLoad);
        }
        setCurrentTaskCallback(prepareUnlockSelectionResume);
        D_8013AE88->unk10 |= 1;
    }
}

INCLUDE_ASM("asm/nonmatchings/main_tail_ui", func_800A5050);

void resumeUnlockSelectionTask(void) {
    setSecondaryDisplayTaskMode(0);
    setCurrentTaskCallback(func_800A499C);
}

void prepareUnlockSelectionResume(void) {
    setSecondaryDisplayTaskMode(3);
    D_8013B10C = -1;
    func_8002FA58(2);
    setCurrentTaskCallback(resumeUnlockSelectionTask);
    D_8013AE88->unk10 |= 2;
}

INCLUDE_ASM("asm/nonmatchings/main_tail_ui", func_800A525C);

void startUnlockResourceLoad(void) {
    resetPendingCdRequests();
    D_8013AFE4 &= ~0x100;
    createResourceLoadTask();
    setCurrentTaskCallback(func_800A525C);
}

void initializeNumberAdjustmentTask(void) {
    NumberAdjustmentState *state;
    s16 i;
    D_8013AE98->data1C++;
    if (D_8013AE88->resource14 != 0xFFFF) {
        for (;;) {}
    }
    D_8013AE88->resource14 = allocateTaskResource();
    state = (NumberAdjustmentState *)&D_800D9504[D_8013AE88->resource14];
    state->value = D_8013AFD0;
    for (i = 0; i < 3; i++) {
        state->unk20[i] = 0;
        state->unk2C[i] = 0;
    }
    state->active = 1;
    func_8002FA58(0x17);
    setCurrentTaskCallback(func_800A53BC);
    D_8013AE88->unk10 |= 2;
}

INCLUDE_ASM("asm/nonmatchings/main_tail_ui", func_800A53BC);

INCLUDE_ASM("asm/nonmatchings/main_tail_ui", func_800A5418);

void drawNumberAdjustmentArrow(s16 direction, GsOT *ot, u16 depth) {
    POLY_F3 *triangle = &gNumberAdjustmentArrow;
    setRGB0(triangle, 0xC0, 0x20, 0);
    if (direction == 0) {
        setXY3(triangle, -46, -32, -40, -44, -34, -32);
    } else {
        setXY3(triangle, -46, -8, -40, 4, -34, -8);
    }
    GsSortPoly(triangle, ot, depth);
}

INCLUDE_ASM("asm/nonmatchings/main_tail_ui", func_800A5630);

INCLUDE_ASM("asm/nonmatchings/main_tail_ui", func_800A59B8);

void func_800A5AD4(void) {
}

void func_800A5ADC(void) {
}

void func_800A5AE4(void) {
}

void func_800A5AEC(void) {
}

void func_800A5AF4(void) {
}

INCLUDE_ASM("asm/nonmatchings/main_tail_ui", func_800A5AFC);

s16 parseDecimalS16(const u8 *text, s16 *result) {
    s16 value = 0;
    s16 count = 0;
    u16 digit;
    while ((u32)(*text - '0') < 10) {
        count++;
        digit = *text++;
        value = value * 10 + (digit - (u16)'0');
    }
    *result = value;
    return count;
}

void resetTextEntryStates(TextControlState *control, TextResourceState *resource) {
    s16 i;
    if (control != NULL) {
        control->unk00 = 0;
        control->unk02 = 1;
        control->unk04 = 0;
        control->unk08 = 0;
        control->unk0C = 0;
        for (i = 0; i < 5; i++) {
            control->unk18[i] = 0;
        }
    }
    if (resource != NULL) {
        for (i = 0; i < 5; i++) {
            resource->unk04[i] = 0;
            resource->unk18[i] = 0;
        }
    }
}

void resetTextEntryPool(void) {
    TextEntry *previous = NULL;
    TextEntry *entry = (TextEntry *)0x80180008;
    u16 i;
    D_8013B5D0 = entry;
    for (i = 0; i < 295; i++) {
        entry->previous = previous;
        previous = entry;
        previous->unk04 = 0;
        previous->unk06 = 0;
        previous->unk08 = 0;
        previous->unk0A = 0;
        previous->next = previous + 1;
        entry++;
    }
    entry[-1].next = NULL;
    D_8013B5D4 = NULL;
}

INCLUDE_ASM("asm/nonmatchings/main_tail_ui", func_800A5F00);

TextEntry *findKeyedListNode(u32 key, u32 value) {
    TextEntry *node = D_8013B5D4;
    TextEntry *result = NULL;
    while (node != NULL) {
        if (key == (u32)node->text && value == node->unk04) {
            result = node;
            break;
        }
        node = node->next;
        result = NULL;
    }
    return result;
}

TextEntry *findTextEntryInsertionPoint(u32 key, u32 value) {
    TextEntry *node = D_8013B5D4;
    while (node != NULL) {
        if (key == (u32)node->text && value < node->unk04) {
            break;
        }
        if (key < (u32)node->text) {
            break;
        }
        node = node->next;
    }
    return node;
}

INCLUDE_ASM("asm/nonmatchings/main_tail_ui", func_800A5FE8);

s16 advanceTextLine(const s8 **text, s16 limit) {
    const s8 *p = *text;
    s16 count = 0;
    while (*p != 0) {
        if (count >= limit) {
            if (*p == '\r' && p[1] == '\n') {
                p += 2;
            }
            break;
        }
        if (*p == '\r' && p[1] == '\n') {
            p += 2;
            break;
        }
        if (*p & 0x80) {
            count++;
            p += 2;
        } else if (*p == ' ') {
            p++;
            count++;
            if (*p != ' ') {
                continue;
            }
            p++;
            continue;
        } else {
            p += 2;
        }
    }
    *text = p;
    return count;
}

void playMusicSequence(s16 count, u8 left, u8 right) {
    if (gMusicSequenceActive) {
        D_8013AF30 = left << 8;
        SsSeqSetVol(gMusicSequenceId, left, right);
        SsSeqPlay(gMusicSequenceId, 1, count);
    }
}

INCLUDE_ASM("asm/nonmatchings/main_tail_ui", func_800A624C);

void initializeLoadedTextEntries(void) {
    TextEntryListHeader *header = (TextEntryListHeader *)0x80180000;
    TextEntry *freeEntries = header->freeEntries;
    TextEntry *activeEntries = header->activeEntries;
    D_8013B5D0 = freeEntries;
    D_8013B5D4 = activeEntries;
    func_800A5AFC();
    setCurrentTaskCallback(func_800A6548);
}

INCLUDE_ASM("asm/nonmatchings/main_tail_ui", func_800A6548);

INCLUDE_ASM("asm/nonmatchings/main_tail_ui", func_800A6684);

INCLUDE_ASM("asm/nonmatchings/main_tail_ui", func_800A68B4);

void initializeSecondaryResourceTask(void) {
    if (D_8013AE98->resource16 == 0xFFFF) {
        while (1) {
        }
    }
    D_8013AE88->data18 = 0;
    setCurrentTaskCallback(func_800A6A8C);
    D_8013AE88->unk10 |= 2;
}

INCLUDE_ASM("asm/nonmatchings/main_tail_ui", func_800A6A8C);

INCLUDE_ASM("asm/nonmatchings/main_tail_ui", func_800A6CC4);

void setSecondaryTextEntryTask(TextEntry *entry) {
    TextTaskResource *resource;
    if (D_8013AE88->resource16 == 0xFFFF) {
        while (1) {}
    }
    resource = (TextTaskResource *)&D_800D9504[D_8013AE88->resource16];
    if (entry == NULL) {
        resource->task = NULL;
    } else {
        func_80014874(func_800A6E88);
        D_8013AE8C->resource14 = allocateTaskResource();
        ((TextTaskResource *)&D_800D9504[D_8013AE8C->resource14])->entry = entry;
        D_8013AE8C->data18 = 0;
        resource->task = D_8013AE8C;
    }
}

INCLUDE_ASM("asm/nonmatchings/main_tail_ui", func_800A6E88);

INCLUDE_ASM("asm/nonmatchings/main_tail_ui", func_800A73BC);

void finishMusicTaskAtDeadline(void) {
    MusicOwnerState *owner;
    TaskResourceRecord *state;
    if (D_8013AE98->resource16 == 0xFFFF) {
        while (1) {}
    }
    owner = (MusicOwnerState *)&D_800D9504[D_8013AE98->resource16];
    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    state = &D_800D9504[D_8013AE88->resource14];
    if (owner->musicTask == D_8013AE88) {
        if (owner->tick < state->unk04) {
            return;
        }
        owner->musicTask = NULL;
        if (owner->completionTick < state->unk04) {
            owner->completionTick = state->unk04;
        }
    }
    func_800149A8();
}

INCLUDE_ASM("asm/nonmatchings/main_tail_ui", func_800A7664);

INCLUDE_ASM("asm/nonmatchings/main_tail_ui", func_800A77B0);

INCLUDE_ASM("asm/nonmatchings/main_tail_ui", func_800A7928);

void stopTextSoundVoices(void) {
    s16 i;
    for (i = 0; i < 8; i++) {
        if (gTextSoundSlots[i].voice != 0xFF) {
            func_800187B8(gTextSoundSlots[i].voice);
            gTextSoundSlots[i].voice = 0xFF;
            gTextSoundSlots[i].soundKey = 0;
        }
    }
}

INCLUDE_ASM("asm/nonmatchings/main_tail_ui", func_800A7B9C);

void signalFlag9WhenTaskCountdownExpires(void) {
    if (--D_8013AE88->data18 == 0) {
        D_8013AFE4 |= 0x200;
        func_800149A8();
    }
}

void startUnlockNotification(s16 index) {
    if ((u16)index < 11 && (s16)getUnlockNotificationFlag(index) == 0) {
        D_8013AF3A = index;
        D_8013AFE4 &= ~0x200;
        setUnlockNotificationFlag(index);
        func_80014874(func_800A624C);
    } else {
        D_8013AFE4 |= 0x200;
    }
}

INCLUDE_ASM("asm/nonmatchings/main_tail_ui", func_800A7D8C);

INCLUDE_ASM("asm/nonmatchings/main_tail_ui", func_800A7F84);

INCLUDE_ASM("asm/nonmatchings/main_tail_ui", func_800A8044);

INCLUDE_ASM("asm/nonmatchings/main_tail_ui", func_800A8104);
