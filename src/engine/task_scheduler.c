#include "input_scheduler_internal.h"

GameTask gGameTasks[200] __attribute__((section(".data.game_tasks"))) = {0};

GameTask *D_8013AE88 __attribute__((section(".sdata.task_scheduler"))) = 0;

GameTask *D_8013AE8C __attribute__((section(".sdata.task_scheduler"))) = 0;

GameTask *D_8013AE90 __attribute__((section(".sdata.task_scheduler"))) = 0;

GameTask *D_8013AE94 __attribute__((section(".sdata.task_scheduler"))) = 0;

GameTask *D_8013AE98 __attribute__((section(".sdata.task_scheduler"))) = 0;

GameTask *D_8013AE9C __attribute__((section(".sdata.task_scheduler"))) = 0;

GameTask *D_8013AEA0 __attribute__((section(".sdata.task_scheduler"))) = 0;

GameTask *D_8013AEA4 __attribute__((section(".sdata.task_scheduler"))) = 0;

GameTask *D_8013AEA8 __attribute__((section(".sdata.task_scheduler"))) = 0;

GameTask *D_8013AEAC __attribute__((section(".sdata.task_scheduler"))) = 0;

GameTask *D_8013AEB0 __attribute__((section(".sdata.task_scheduler"))) = 0;

u16 D_8013AEB4 __attribute__((section(".sdata.task_scheduler"))) = 0;

static u16 sTaskSchedulerPadding __attribute__((section(".sdata.task_scheduler"))) = 0;

u16 gTaskResourceFreeBits[16] __attribute__((section(".data.task_resource_bitmap"))) = {0};

TaskCallback gInitialTaskCallbacks[6] __attribute__((section(".data.initial_task_callbacks"))) = {
    initializeGameOptionTask,
    func_8001E0C0,
    func_8001E0C0,
    func_8001E148,
    func_8001E148,
    func_8001E1B8,
};

void initializeTaskScheduler(void) {
    u16 i;
    for (i = 0; i < 6; i++) {
        gGameTasks[i].next = &gGameTasks[i + 1];
        gGameTasks[i].previous = &gGameTasks[i - 1];
        gGameTasks[i].parent = 0;
        gGameTasks[i].callback = gInitialTaskCallbacks[i];
        gGameTasks[i].unk10 = 0;
        gGameTasks[i].depth = 0;
        gGameTasks[i].resource16 = -1;
        gGameTasks[i].resource14 = -1;
    }
    gGameTasks[0].previous = 0;
    gGameTasks[5].next = 0;
    D_8013AE90 = &gGameTasks[1];
    D_8013AE94 = &gGameTasks[2];
    D_8013AE9C = &gGameTasks[6];
    for (i = 6; i < 200; i++) {
        gGameTasks[i].next = &gGameTasks[i + 1];
        gGameTasks[i].previous = 0;
        gGameTasks[i].parent = 0;
        gGameTasks[i].callback = 0;
        gGameTasks[i].unk10 = -1;
        gGameTasks[i].depth = 0;
        gGameTasks[i].resource16 = -1;
        gGameTasks[i].resource14 = -1;
    }
    D_8013AEA0 = &gGameTasks[199];
    D_8013AEB4 = 193;
    for (i = 0; i < 16; i++) {
        gTaskResourceFreeBits[i] = -1;
    }
}

INCLUDE_ASM("asm/nonmatchings/engine/task_scheduler", func_800147B8);

INCLUDE_ASM("asm/nonmatchings/engine/task_scheduler", func_80014838);

INCLUDE_ASM("asm/nonmatchings/engine/task_scheduler", func_80014874);

INCLUDE_ASM("asm/nonmatchings/engine/task_scheduler", func_800148B4);

INCLUDE_ASM("asm/nonmatchings/engine/task_scheduler", func_800148F4);

INCLUDE_ASM("asm/nonmatchings/engine/task_scheduler", func_80014930);

INCLUDE_ASM("asm/nonmatchings/engine/task_scheduler", func_8001496C);

INCLUDE_ASM("asm/nonmatchings/engine/task_scheduler", func_800149A8);

INCLUDE_ASM("asm/nonmatchings/engine/task_scheduler", func_800149E0);

void deletePendingTaskSubtree(void) {
    u32 depth = D_8013AE88->depth;
    while (1) {
        D_8013AEB4++;
        if (D_8013AEB0->resource14 != 0xFFFF) {
            releaseTaskResource(D_8013AEB0->resource14);
        }
        if (D_8013AEB0->resource16 != 0xFFFF) {
            releaseTaskResource(D_8013AEB0->resource16);
        }
        D_8013AEB0->unk10 = 0xFFFF;
        D_8013AEA4 = D_8013AEB0->next;
        if (D_8013AEA4 != 0 && D_8013AEA4->depth > depth) {
            D_8013AEB0 = D_8013AEA4;
        } else {
            break;
        }
    }
    D_8013AEA0->next = D_8013AEAC;
    D_8013AEA4 = D_8013AEB0->next;
    D_8013AEA8 = D_8013AEAC->previous;
    D_8013AEA0 = D_8013AEB0;
    D_8013AEA8->next = D_8013AEA4;
    if (D_8013AEA4 != 0) {
        D_8013AEA4->previous = D_8013AEA8;
    }
}

INCLUDE_ASM("asm/nonmatchings/engine/task_scheduler", func_80014B14);

void setCurrentTaskCallback(TaskCallback callback) { D_8013AE88->callback = callback; }

INCLUDE_ASM("asm/nonmatchings/engine/task_scheduler", func_80014B58);

void deleteCurrentTask(void) { func_800149A8(); }

u32 allocateTaskResource(void) {
    u16 i, bit;
    u32 mask;
    for (i = 0; i < 16; i++) {
        if (gTaskResourceFreeBits[i]) {
            for (mask = 1, bit = 0; bit < 16; bit++) {
                if (gTaskResourceFreeBits[i] & mask) {
                    gTaskResourceFreeBits[i] ^= mask;
                    return i * 16 + bit;
                }
                mask = (u16)mask << 1;
            }
        }
    }
    return 0;
}

void releaseTaskResource(u16 resource) {
    u32 index = resource >> 4;
    resource %= 16;
    gTaskResourceFreeBits[index] |= 1 << resource;
}
