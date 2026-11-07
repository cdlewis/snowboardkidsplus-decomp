#include "selection_tasks_internal.h"

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

INCLUDE_ASM("asm/nonmatchings/menu/number_adjustment", func_800A53BC);

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
        func_8002F394();
        D_8013AE88->data18 = 0;
        setCurrentTaskCallback(finishNumberAdjustmentTask);
    }
    func_800A5630();
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

INCLUDE_ASM("asm/nonmatchings/menu/number_adjustment", func_800A5630);

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
        if (func_800707F0(0) != 0) break;
    case 2:
        D_8013AE98->data1C--;
        func_800149A8();
        return;
    }
    func_800A5630();
}
