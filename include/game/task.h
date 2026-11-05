#pragma once

#include "decomp/types.h"

typedef void (*TaskCallback)(void);

/* The scheduler allocates 0x20-byte records and links children after parents. */
typedef struct GameTask {
    TaskCallback callback;
    struct GameTask *parent;
    struct GameTask *next;
    struct GameTask *previous;
    u16 unk10;
    u8 depth;
    u8 unk13;
    u16 resource14;
    u16 resource16;
    u16 data18;
    u16 data1A;
    u16 data1C;
    u16 data1E;
} GameTask;

/* One set bit for each available task resource. */
extern u16 gTaskResourceFreeBits[16];

/* Scheduler storage remains in its original data object. */
/* Some task flows address this pointer absolutely; the scheduler uses $gp. */
#ifdef TASK_ABSOLUTE_ADDRESSING
extern GameTask *D_8013AE88 __attribute__((section(".data")));
#else
extern GameTask *D_8013AE88;
#endif
extern GameTask D_800D7C84;

s16 func_80014930(GameTask *parent, TaskCallback callback);
void func_800149A8(void);
void setCurrentTaskCallback(TaskCallback callback);
void deleteCurrentTask(void);
void releaseTaskResource(u16 resource);

void setTaskSecondaryResourceData(GameTask *task, const u16 *data);

s32 isTaskResourceTransitionComplete(s16 index);
