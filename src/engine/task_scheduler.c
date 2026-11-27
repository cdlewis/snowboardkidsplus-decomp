#include "input_scheduler_internal.h"
#include "game/engine/game_options.h"
#include "game/engine/task.h"

GameTask gGameTasks[200] __attribute__((section(".data.game_tasks"))) = {0};

GameTask *gCurrentTask __attribute__((section(".sdata.task_scheduler"))) = 0;

GameTask *gCreatedTask __attribute__((section(".sdata.task_scheduler"))) = 0;

GameTask *D_8013AE90 __attribute__((section(".sdata.task_scheduler"))) = 0;

GameTask *D_8013AE94 __attribute__((section(".sdata.task_scheduler"))) = 0;

GameTask *gCurrentTaskParent __attribute__((section(".sdata.task_scheduler"))) = 0;

GameTask *D_8013AE9C __attribute__((section(".sdata.task_scheduler"))) = 0;

GameTask *D_8013AEA0 __attribute__((section(".sdata.task_scheduler"))) = 0;

GameTask *D_8013AEA4 __attribute__((section(".sdata.task_scheduler"))) = 0;

GameTask *D_8013AEA8 __attribute__((section(".sdata.task_scheduler"))) = 0;

GameTask *D_8013AEAC __attribute__((section(".sdata.task_scheduler"))) = 0;

GameTask *D_8013AEB0 __attribute__((section(".sdata.task_scheduler"))) = 0;

u16 D_8013AEB4 __attribute__((section(".sdata.task_scheduler"))) = 0;

static u16 sTaskSchedulerPadding __attribute__((section(".sdata.task_scheduler"))) = 0;

u16 gTaskMemoryFreeBits[16] __attribute__((section(".data.task_resource_bitmap"))) = {0};

TaskCallback gInitialTaskCallbacks[6] __attribute__((section(".data.initial_task_callbacks"))) = {
    initializeGameOptionTask,
    armTaskCleanupOnFlagAFAABit6,
    armTaskCleanupOnFlagAFAABit6,
    armTaskCallbackResetOnFlag3,
    armTaskCallbackResetOnFlag3,
    armSavedStateRestoreOnFlag3,
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
        gTaskMemoryFreeBits[i] = -1;
    }
}

void insertChildTaskFromFreeList(TaskCallback callback) {
    GameTask *next;
    D_8013AEB4--;
    gCreatedTask = D_8013AE9C;
    D_8013AE9C = D_8013AE9C->next;
    next = gCurrentTask->next;
    gCreatedTask->next = next;
    D_8013AEA4 = next;
    if (next != 0) {
        next->previous = gCreatedTask;
    }
    gCurrentTask->next = gCreatedTask;
    gCreatedTask->previous = gCurrentTask;
    gCreatedTask->parent = gCurrentTask;
    gCreatedTask->callback = callback;
    gCreatedTask->depth = gCurrentTask->depth + 1;
    gCreatedTask->unk10 = 0;
    gCreatedTask->resource16 = 0xFFFF;
    gCreatedTask->resource14 = 0xFFFF;
}

s32 createChildTask(TaskCallback callback) {
    if (D_8013AEB4 == 0) {
        gCreatedTask = D_8013AEA0;
        return 0;
    }
    insertChildTaskFromFreeList(callback);
    return 1;
}

s32 createChildTaskWithReserve(TaskCallback callback) {
    if (D_8013AEB4 <= 10) {
        gCreatedTask = D_8013AEA0;
        return 0;
    }
    insertChildTaskFromFreeList(callback);
    return 1;
}

s32 tryCreateChildTaskWith20SlotsReserved(TaskCallback callback) {
    if (D_8013AEB4 <= 20) {
        /* Expose the free-list tail as the fallback task. */
        gCreatedTask = D_8013AEA0;
        return 0;
    }

    insertChildTaskFromFreeList(callback);
    return 1;
}

s16 createChildTaskForParent(GameTask *parent, TaskCallback callback) {
    GameTask *previous = gCurrentTask;
    s16 result;

    gCurrentTask = parent;
    result = createChildTask(callback);
    gCurrentTask = previous;
    return result;
}

s16 createChildTaskForParentWithReserve(GameTask *parent, TaskCallback callback) {
    GameTask *previous = gCurrentTask;
    s16 result;

    gCurrentTask = parent;
    result = createChildTaskWithReserve(callback);
    gCurrentTask = previous;
    return result;
}

s16 tryCreateChildTaskForParentWith20SlotsReserved(GameTask *parent, TaskCallback callback) {
    GameTask *previous = gCurrentTask;
    s16 result;

    gCurrentTask = parent;
    result = tryCreateChildTaskWith20SlotsReserved(callback);
    gCurrentTask = previous;
    return result;
}

void deleteCurrentTaskSubtree(void) {
    GameTask *task = gCurrentTask;

    D_8013AEB0 = task;
    D_8013AEAC = task;
    deletePendingTaskSubtree();
    /* Resume traversal from the predecessor of the removed subtree. */
    gCurrentTask = D_8013AEA8;
}

void deleteCurrentTaskDescendants(void) {
    if (currentTaskHasChildren()) {
        GameTask *firstDescendant = gCurrentTask->next;

        D_8013AEB0 = firstDescendant;
        D_8013AEAC = firstDescendant;
        deletePendingTaskSubtree();
    }
}

void deletePendingTaskSubtree(void) {
    u32 depth = gCurrentTask->depth;
    while (1) {
        D_8013AEB4++;
        if (D_8013AEB0->resource14 != 0xFFFF) {
            freeTaskMemoryBlock(D_8013AEB0->resource14);
        }
        if (D_8013AEB0->resource16 != 0xFFFF) {
            freeTaskMemoryBlock(D_8013AEB0->resource16);
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

u16 currentTaskHasChildren(void) {
    GameTask *current = gCurrentTask;
    u16 result;

    D_8013AEA4 = current->next;
    if (D_8013AEA4 != NULL) {
        result = D_8013AEA4->depth > current->depth;
    } else {
        result = 0;
    }
    return result;
}

void setCurrentTaskCallback(TaskCallback callback) { gCurrentTask->callback = callback; }

void deleteCurrentTaskWhenCdCommandQueueIdle(void) {
    if (isCdCommandQueueIdle() == 1) {
        deleteCurrentTaskSubtree();
    }
}

void deleteCurrentTask(void) { deleteCurrentTaskSubtree(); }

u32 allocateTaskMemoryBlock(void) {
    u16 i, bit;
    u32 mask;
    for (i = 0; i < 16; i++) {
        if (gTaskMemoryFreeBits[i]) {
            for (mask = 1, bit = 0; bit < 16; bit++) {
                if (gTaskMemoryFreeBits[i] & mask) {
                    gTaskMemoryFreeBits[i] ^= mask;
                    return i * 16 + bit;
                }
                mask = (u16)mask << 1;
            }
        }
    }
    return 0;
}

void freeTaskMemoryBlock(u16 blockIndex) {
    u32 index = blockIndex >> 4;
    blockIndex %= 16;
    gTaskMemoryFreeBits[index] |= 1 << blockIndex;
}
