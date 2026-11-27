#include "input_scheduler_internal.h"
#include "game/engine/game_options.h"
#include "game/engine/task.h"

GameTask gGameTasks[200] __attribute__((section(".data.game_tasks"))) = {0};

GameTask *gCurrentTask __attribute__((section(".sdata.task_scheduler"))) = 0;

GameTask *gCreatedTask __attribute__((section(".sdata.task_scheduler"))) = 0;

GameTask *gPauseTaskResumeAnchor __attribute__((section(".sdata.task_scheduler"))) = 0;

GameTask *D_8013AE94 __attribute__((section(".sdata.task_scheduler"))) = 0;

GameTask *gCurrentTaskParent __attribute__((section(".sdata.task_scheduler"))) = 0;

GameTask *gTaskFreeListHead __attribute__((section(".sdata.task_scheduler"))) = 0;

GameTask *gTaskFreeListTail __attribute__((section(".sdata.task_scheduler"))) = 0;

GameTask *gTaskNextScratch __attribute__((section(".sdata.task_scheduler"))) = 0;

GameTask *gDeletedTaskPredecessor __attribute__((section(".sdata.task_scheduler"))) = 0;

GameTask *gTaskDeletionFirst __attribute__((section(".sdata.task_scheduler"))) = 0;

GameTask *gTaskDeletionCursor __attribute__((section(".sdata.task_scheduler"))) = 0;

u16 gAvailableTaskSlots __attribute__((section(".sdata.task_scheduler"))) = 0;

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
    gPauseTaskResumeAnchor = &gGameTasks[1];
    D_8013AE94 = &gGameTasks[2];
    gTaskFreeListHead = &gGameTasks[6];
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
    gTaskFreeListTail = &gGameTasks[199];
    gAvailableTaskSlots = 193;
    for (i = 0; i < 16; i++) {
        gTaskMemoryFreeBits[i] = -1;
    }
}

void insertChildTaskFromFreeList(TaskCallback callback) {
    GameTask *next;
    gAvailableTaskSlots--;
    gCreatedTask = gTaskFreeListHead;
    gTaskFreeListHead = gTaskFreeListHead->next;
    next = gCurrentTask->next;
    gCreatedTask->next = next;
    gTaskNextScratch = next;
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
    if (gAvailableTaskSlots == 0) {
        gCreatedTask = gTaskFreeListTail;
        return 0;
    }
    insertChildTaskFromFreeList(callback);
    return 1;
}

s32 createChildTaskWithReserve(TaskCallback callback) {
    if (gAvailableTaskSlots <= 10) {
        gCreatedTask = gTaskFreeListTail;
        return 0;
    }
    insertChildTaskFromFreeList(callback);
    return 1;
}

s32 tryCreateChildTaskWith20SlotsReserved(TaskCallback callback) {
    if (gAvailableTaskSlots <= 20) {
        /* Expose the free-list tail as the fallback task. */
        gCreatedTask = gTaskFreeListTail;
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

    gTaskDeletionCursor = task;
    gTaskDeletionFirst = task;
    deletePendingTaskSubtree();
    /* Resume traversal from the predecessor of the removed subtree. */
    gCurrentTask = gDeletedTaskPredecessor;
}

void deleteCurrentTaskDescendants(void) {
    if (currentTaskHasChildren()) {
        GameTask *firstDescendant = gCurrentTask->next;

        gTaskDeletionCursor = firstDescendant;
        gTaskDeletionFirst = firstDescendant;
        deletePendingTaskSubtree();
    }
}

void deletePendingTaskSubtree(void) {
    u32 depth = gCurrentTask->depth;
    while (1) {
        gAvailableTaskSlots++;
        if (gTaskDeletionCursor->resource14 != 0xFFFF) {
            freeTaskMemoryBlock(gTaskDeletionCursor->resource14);
        }
        if (gTaskDeletionCursor->resource16 != 0xFFFF) {
            freeTaskMemoryBlock(gTaskDeletionCursor->resource16);
        }
        gTaskDeletionCursor->unk10 = 0xFFFF;
        gTaskNextScratch = gTaskDeletionCursor->next;
        if (gTaskNextScratch != 0 && gTaskNextScratch->depth > depth) {
            gTaskDeletionCursor = gTaskNextScratch;
        } else {
            break;
        }
    }
    gTaskFreeListTail->next = gTaskDeletionFirst;
    gTaskNextScratch = gTaskDeletionCursor->next;
    gDeletedTaskPredecessor = gTaskDeletionFirst->previous;
    gTaskFreeListTail = gTaskDeletionCursor;
    gDeletedTaskPredecessor->next = gTaskNextScratch;
    if (gTaskNextScratch != 0) {
        gTaskNextScratch->previous = gDeletedTaskPredecessor;
    }
}

u16 currentTaskHasChildren(void) {
    GameTask *current = gCurrentTask;
    u16 result;

    gTaskNextScratch = current->next;
    if (gTaskNextScratch != NULL) {
        result = gTaskNextScratch->depth > current->depth;
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
