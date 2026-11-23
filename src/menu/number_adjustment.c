#include "selection_tasks_internal.h"
#include "game/audio/music.h"
#include "game/engine/game_task_scheduler.h"
#include "game/engine/task.h"
#include "game/graphics/graphics.h"
#include "game/io/cd_queue.h"
#include "game/io/controller.h"
#include "game/menu/number_selection_types.h"
#include "game/menu/selected_records.h"
#include "psx/libgpu.h"
#include "psx/libgs.h"

extern s16 gFrameCounter __attribute__((section(".data")));
extern u16 D_8013AF78 __attribute__((section(".data")));
extern GameTask *D_8013AE88 __attribute__((section(".data")));
extern GameTask *D_8013AE98 __attribute__((section(".data")));

extern u16 D_8013AFD0 __attribute__((section(".data")));

const u16 gNumberAdjustmentLabel[]
    __attribute__((section(".data.number_adjustment_label"))) = {
    0x00F7, 0x00CE, 0x0093, 0x00A1, 0x00D2, 0,
};

/* The null entry at the start of the following pointer table terminates this text. */
const u16 gNumberAdjustmentInstructions[]
    __attribute__((section(".data.number_adjustment_instructions"))) = {
    0x00F7, 0x00F1, 0x00DD, 0x00BF, 0x00E7, 0x00E8, 0x0049, 0x002C,
    0x00CE, 0x800A, 0x00E3, 0x00C0, 0x00CE, 0x0039, 0x015A, 0x0084,
    0x0052, 0x0070, 0x00BF, 0x0097, 0x009E, 0x00A5, 0x009A, 0x800A,
    0x00CB, 0x009D, 0x0094, 0x0033,
};

const u16 gNumberAdjustmentDigitTexts[9][2]
    __attribute__((section(".data.number_adjustment_digit_texts"))) = {
    {2, 0}, {3, 0}, {4, 0}, {5, 0}, {6, 0}, {7, 0}, {8, 0}, {9, 0}, {10, 0},
};

const u16 *gNumberAdjustmentValueTexts[10]
    __attribute__((section(".data.number_adjustment_value_texts"))) = {
    NULL,
    gNumberAdjustmentDigitTexts[0],
    gNumberAdjustmentDigitTexts[1],
    gNumberAdjustmentDigitTexts[2],
    gNumberAdjustmentDigitTexts[3],
    gNumberAdjustmentDigitTexts[4],
    gNumberAdjustmentDigitTexts[5],
    gNumberAdjustmentDigitTexts[6],
    gNumberAdjustmentDigitTexts[7],
    gNumberAdjustmentDigitTexts[8],
};

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
    queueMenuResourceReads(0x17);
    setCurrentTaskCallback(func_800A53BC);
    D_8013AE88->unk10 |= 2;
}

/* DECOMP_ATTEMPT: inline 19/23 */
#ifdef NON_MATCHING
extern u8 D_8012D478[];
void setTaskBackgroundMode(s32 mode, s32 value1, s32 value2);
void copyIndexedTaskImagesToVram(s16 index);
void updateNumberAdjustmentTask(void);

void func_800A53BC(void) {
    s16 index;
    setTaskBackgroundMode(2, -0x1000, 0x1000);
    setSecondaryDisplayTaskMode(0);
    index = D_8012D478[D_8013AFCE];
    copyIndexedTaskImagesToVram(index);
    setCurrentTaskCallback(updateNumberAdjustmentTask);
}
#else
INCLUDE_ASM("asm/nonmatchings/menu/number_adjustment", func_800A53BC);
#endif

void updateNumberAdjustmentTask(void) {
    NumberAdjustmentState *state;
    s16 previous;
    u16 buttons;
    if (D_8013AE88->resource14 == 0xFFFF) {
        for (;;) {}
    }
    state = (NumberAdjustmentState *)&D_800D9504[D_8013AE88->resource14];
    buttons = D_8013AEE8[0] | D_8013AEE8[1];
    previous = state->value;
    if ((buttons & 0x1000) && state->value > 1) {
        state->value--;
    }
    if ((buttons & 0x4000) && state->value < 9) {
        state->value++;
    }
    if (previous != state->value) {
        playSoundEffect(0, 0x100, 0x20);
    }
    if (buttons & 0xB0) {
        playSoundEffect(0, 0, 0x20);
        D_8013AFD0 = state->value;
        state->countdown = 20;
        state->active = 0;
        D_8013AE98->data18 = 0x17;
        D_8013AE88->data18 = 0;
        setCurrentTaskCallback(finishNumberAdjustmentTask);
    }
    if (buttons & 0x40) {
        playSoundEffect(0, 0, 0x20);
        D_8013AE98->data18 = 8;
        queueBackgroundTextureArchiveRead();
        D_8013AE88->data18 = 0;
        setCurrentTaskCallback(finishNumberAdjustmentTask);
    }
    drawNumberAdjustmentPanel();
}

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

void drawNumberAdjustmentPanel(void) {
    NumberAdjustmentState *state;
    s32 scroll;
    s32 palette;

    if (D_8013AE88->resource14 == 0xFFFF) {
        for (;;) {}
    }
    state = (NumberAdjustmentState *)&D_800D9504[D_8013AE88->resource14];
    setSpriteColor(128, 128, 128);
    setTexturedQuadColor(128, 128, 128);
    scroll = state->unk20[0] / 4096;
    func_8006C998(-76, scroll - 91, 296, 0, -1, &D_800EBFF0[D_8013AF78], 4);
    palette = 13;
    func_8006C998(-84, scroll - 96, 295, 1, -1, &D_800EBFF0[D_8013AF78], 4);
    if (state->countdown > 0) {
        if (state->countdown & 1) {
            palette = 8;
        }
        state->countdown--;
    }
    func_8006EF30(-48, -28, 30, gNumberAdjustmentValueTexts[state->value],
                   palette, 1, &D_800EBFF0[D_8013AF78], 4);
    if (state->active) {
        if (state->value > 1 && (gFrameCounter & 8)) {
            drawNumberAdjustmentArrow(0, &D_800EBFF0[D_8013AF78], 4);
        }
        if (state->value < 9 && (gFrameCounter & 8)) {
            drawNumberAdjustmentArrow(1, &D_800EBFF0[D_8013AF78], 4);
        }
    }
    func_8006EF30(-32, -28, 30, gNumberAdjustmentLabel,
                   palette, 1, &D_800EBFF0[D_8013AF78], 4);
    drawDefaultTexturedPanel(-64, -52, 128, 64, 5, 0, &D_800EBFF0[D_8013AF78], 4);
    func_8006EF30(-104, 34, 30, gNumberAdjustmentInstructions,
                   15, 1, &D_800EBFF0[D_8013AF78], 4);
    drawDefaultTexturedPanel(-120, 26, 240, 64, 5, 0, &D_800EBFF0[D_8013AF78], 4);
}

void finishNumberAdjustmentTask(void) {
    NumberAdjustmentState *state;
    if (D_8013AE88->resource14 == 0xFFFF) {
        for (;;) {}
    }
    state = (NumberAdjustmentState *)&D_800D9504[D_8013AE88->resource14];
    switch (D_8013AE88->data18) {
    case 0:
        if (state->countdown == 0) {
            if (D_8013AE98->data18 == 0x17) {
                beginMode2SequenceAndFadeMusic();
                D_8013AE88->data18 = 1;
            } else {
                D_8013AE88->data18 = 2;
            }
        }
        break;
    case 1:
        if (isTaskStateTransitionBelowMaximum(0) != 0) break;
    case 2:
        D_8013AE98->data1C--;
        deleteCurrentTaskSubtree();
        return;
    }
    drawNumberAdjustmentPanel();
}
