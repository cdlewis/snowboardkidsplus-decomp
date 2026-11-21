#include "input_scheduler_internal.h"
#include "game/engine/game_options.h"
#include "game/engine/task.h"

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
    armTaskCleanupOnFlagAFAABit6,
    armTaskCleanupOnFlagAFAABit6,
    armTaskCallbackResetOnFlag3,
    armTaskCallbackResetOnFlag3,
    armSavedStateRestoreOnFlag3,
};

void func_800147B8(TaskCallback callback);

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

/* DECOMP_ATTEMPT: inline 30/32 */
#ifdef NON_MATCHING
void func_800147B8(TaskCallback callback) {
    GameTask *next;
    D_8013AEB4--;
    D_8013AE8C = D_8013AE9C;
    D_8013AE9C = D_8013AE9C->next;
    next = D_8013AE88->next;
    D_8013AE8C->next = next;
    D_8013AEA4 = next;
    if (next != 0) {
        next->previous = D_8013AE8C;
    }
    D_8013AE88->next = D_8013AE8C;
    D_8013AE8C->previous = D_8013AE88;
    D_8013AE8C->parent = D_8013AE88;
    D_8013AE8C->callback = callback;
    D_8013AE8C->depth = D_8013AE88->depth + 1;
    D_8013AE8C->unk10 = 0;
    D_8013AE8C->resource16 = 0xFFFF;
    D_8013AE8C->resource14 = 0xFFFF;
}
#else
INCLUDE_ASM("asm/nonmatchings/engine/task_scheduler", func_800147B8);
#endif

s32 createChildTask(TaskCallback callback) {
    if (D_8013AEB4 == 0) {
        D_8013AE8C = D_8013AEA0;
        return 0;
    }
    func_800147B8(callback);
    return 1;
}

s32 createChildTaskWithReserve(TaskCallback callback) {
    if (D_8013AEB4 <= 10) {
        D_8013AE8C = D_8013AEA0;
        return 0;
    }
    func_800147B8(callback);
    return 1;
}

s32 tryCreateChildTaskWith20SlotsReserved(TaskCallback callback) {
    if (D_8013AEB4 <= 20) {
        /* Expose the free-list tail as the fallback task. */
        D_8013AE8C = D_8013AEA0;
        return 0;
    }

    func_800147B8(callback);
    return 1;
}

/* DECOMP_ATTEMPT: inline 10/15 */
#ifdef NON_MATCHING
s16 func_800148F4(GameTask *parent, TaskCallback callback) {
    GameTask *previous = D_8013AE88;
    s16 result;

    D_8013AE88 = parent;
    result = createChildTask(callback);
    D_8013AE88 = previous;
    return result;
}
#else
INCLUDE_ASM("asm/nonmatchings/engine/task_scheduler", func_800148F4);
#endif

/* DECOMP_ATTEMPT: inline 15/15 */
#ifdef NON_MATCHING
s16 func_80014930(GameTask *parent, TaskCallback callback) {
    GameTask *previous = D_8013AE88;
    s16 result;

    D_8013AE88 = parent;
    result = createChildTaskWithReserve(callback);
    D_8013AE88 = previous;
    return result;
}
#else
INCLUDE_ASM("asm/nonmatchings/engine/task_scheduler", func_80014930);
#endif

/* DECOMP_ATTEMPT: inline 10/15 */
#ifdef NON_MATCHING
s32 tryCreateChildTaskWith20SlotsReserved(TaskCallback callback);

s16 func_8001496C(GameTask *parent, TaskCallback callback) {     GameTask *previous = D_8013AE88;     s16 result;      D_8013AE88 = parent;     result = tryCreateChildTaskWith20SlotsReserved(callback);     D_8013AE88 = previous;     return result; }
#else
INCLUDE_ASM("asm/nonmatchings/engine/task_scheduler", func_8001496C);
#endif

void deleteCurrentTaskSubtree(void) {
    GameTask *task = D_8013AE88;

    D_8013AEB0 = task;
    D_8013AEAC = task;
    deletePendingTaskSubtree();
    /* Resume traversal from the predecessor of the removed subtree. */
    D_8013AE88 = D_8013AEA8;
}

void deleteCurrentTaskDescendants(void) {
    if (func_80014B14()) {
        GameTask *firstDescendant = D_8013AE88->next;

        D_8013AEB0 = firstDescendant;
        D_8013AEAC = firstDescendant;
        deletePendingTaskSubtree();
    }
}

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

/* DECOMP_ATTEMPT: inline 9/14 */
#ifdef NON_MATCHING
u16 func_80014B14(void) {
    GameTask *current=D_8013AE88;
    u16 result;
    D_8013AEA4=current->next;
    if (D_8013AEA4) result=D_8013AEA4->depth > current->depth;
    else result=0;
    return result;
}
#else
INCLUDE_ASM("asm/nonmatchings/engine/task_scheduler", func_80014B14);
#endif

void setCurrentTaskCallback(TaskCallback callback) { D_8013AE88->callback = callback; }

void deleteCurrentTaskWhenCdCommandQueueIdle(void) {
    if (isCdCommandQueueIdle() == 1) {
        deleteCurrentTaskSubtree();
    }
}

void deleteCurrentTask(void) { deleteCurrentTaskSubtree(); }

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
