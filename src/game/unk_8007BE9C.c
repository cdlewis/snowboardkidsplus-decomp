#include "common.h"
#define TASK_ABSOLUTE_ADDRESSING
#include "game/task.h"
#include "game/cd_queue.h"
#include "game/graphics.h"
#include "game/linear_random.h"
#include "game/offset_archive.h"
#include "game/music.h"
#include "psx/libsnd.h"

extern u8 D_8013AFE2 __attribute__((section(".data")));

INCLUDE_ASM("asm/nonmatchings/game/unk_8007BE9C", func_8007BE9C);

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

INCLUDE_ASM("asm/nonmatchings/game/unk_8007BE9C", func_8007C7C8);

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
    setCurrentTaskCallback(func_8007CC78);
    D_8013AE88->unk10 |= 2;
}
