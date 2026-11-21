#include "common.h"
#define TASK_ABSOLUTE_ADDRESSING
#include "game/engine/task.h"
#include "game/io/cd_queue.h"
#define TASK_WIDGET_COLORS_ABSOLUTE_ADDRESSING
#include "game/graphics/graphics.h"
#include "game/engine/game_options.h"
#include "game/menu/movement.h"
#include "game/race/player/race_player.h"

INCLUDE_ASM("asm/nonmatchings/unclassified/menu_8007BE9C", func_8007BE9C);

s32 getValidatedSelectionIndex(void) {
    switch (D_8013AFE2) {
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
        return D_8013AFE2;
    }
    return 0;
}

extern char gSelectionMoneyCurrencyLabel[] __attribute__((section(".data")));

void drawSelectionHeaderAndMoneyPanel(void) {
    Movement2D *position;
    u8 playerCount;

    if (D_8013AE88->resource14 == 0xFFFF) {
        for (;;) {}
    }
    position = (Movement2D *)&D_800D9504[D_8013AE88->resource14];
    if (D_8013AE88->data1A == 0) {
        return;
    }
    func_8006C998(position->x / 4096 + 8, position->y / 4096 - 22,
                 0xB1, 0, -1, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
    playerCount = D_8013AFCD;
    if (playerCount != 1) {
        return;
    }
    func_8006C998(position->x / 4096 + 74, position->y / 4096 + 140,
                 0xBD, 10, -1, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
    drawDecimalNumber8x8(position->x / 4096 + 74, position->y / 4096 + 152,
                         6, 0, D_80130BF0[0], 0, -1, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
    drawWrappedText8x8(position->x / 4096 + 122, position->y / 4096 + 152,
                       2, gSelectionMoneyCurrencyLabel, 0, -1, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
    if (D_8013AE88->data1A == playerCount) {
        func_8007240C(position->x / 4096 + 68, position->y / 4096 + 136, 68, 32,
                      D_8013B3A4, D_8013B3A5, D_8013B3A6, 2,
                      &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
    } else {
        func_8007240C(position->x / 4096 + 68, position->y / 4096 + 136, 68, 32,
                      128, 128, 128, 2, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
    }
}

char gSelectionMoneyCurrencyLabel[4] __attribute__((section(".data.selection_money_currency_label"), aligned(4))) = "\x82" "f";

void initializeFlagSelectedCdAwareTask(void) {
    GameTask *task;
    GameTask *parent;
    D_8013AE88->data18 = 1;
    D_8013AE88->data1A = func_8007D47C();
    task = D_8013AE88;
    parent = D_8013AE98;
    parent->data1C++;
    if (task->resource14 != 0xFFFF) {
        for (;;) {}
    }
    D_8013AE88->resource14 = allocateTaskResource();
    if (D_8013AE88->resource16 != 0xFFFF) {
        for (;;) {}
    }
    D_8013AE88->resource16 = allocateTaskResource();
    if (isCdCommandQueueIdle() != 1) {
        setSecondaryDisplayTaskMode(2);
    }
    setCurrentTaskCallback(updateCdAwareCourseSelectionTask);
    D_8013AE88->unk10 |= 2;
}
