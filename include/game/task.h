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

/* Scheduler storage remains in its original data object. */
extern GameTask *D_8013AE88;

void func_800149A8(void);
void setCurrentTaskCallback(TaskCallback callback);
void deleteCurrentTask(void);

void setTaskSecondaryResourceData(GameTask *task, const u16 *data);
