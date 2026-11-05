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
extern GameTask *D_8013AE8C __attribute__((section(".data")));
#else
extern GameTask *D_8013AE88;
extern GameTask *D_8013AE8C;
#endif
extern GameTask D_800D7C84;
/* The resource allocator supplies 256-byte records to task-specific users. */
typedef struct TaskResourceRecord {
    u32 unk00;
    u32 unk04;
    const u16 *unk08;
    u8 unk0C[0xF4];
} TaskResourceRecord;

extern TaskResourceRecord D_800D9504[256];

s16 func_80014930(GameTask *parent, TaskCallback callback);
s32 func_80014874(TaskCallback callback);
void func_800149A8(void);
void func_800149E0(void);
void setCurrentTaskCallback(TaskCallback callback);
void func_800719A8(s16 mode);
void deleteCurrentTask(void);
u32 func_80014BAC(void);
void releaseTaskResource(u16 resource);

void setTaskSecondaryResourceData(GameTask *task, const u16 *data);

s32 isTaskResourceTransitionComplete(s16 index);
void clearControllerFlag5(u16 controller);
s16 hasTaskResource16Value(GameTask *task);

s32 hasTaskStateValue6(s32 id);
void func_80070274(s32 mode, s32 id, u16 duration);

void startIndexedRenderTask(s32 index);
