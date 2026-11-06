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
extern GameTask *D_8013AE98 __attribute__((section(".data")));
extern GameTask *D_8013B304 __attribute__((section(".data")));
#else
extern GameTask *D_8013AE88;
extern GameTask *D_8013AE8C;
extern GameTask *D_8013AE98;
extern GameTask *D_8013B304;
#endif
/* Task transition flags consumed by the main presentation flow. */
#if defined(TASK_TRANSITION_FLAGS_ABSOLUTE_ADDRESSING) || defined(TASK_FLAGS_ABSOLUTE_ADDRESSING)
extern u32 D_8013AFE4 __attribute__((section(".data")));
#else
extern u32 D_8013AFE4;
#endif
extern GameTask gGameTasks[200];
/* The resource allocator supplies 256-byte records to task-specific users. */
typedef struct TaskResourceRecord {
    u32 unk00;
    u32 unk04;
    const u16 *unk08;
    u8 unk0C[0xF4];
} TaskResourceRecord;

extern TaskResourceRecord D_800D9504[256];

void initializeTaskScheduler(void);
/* Initial scheduler callbacks implemented by the main presentation flow. */
void func_8001E0C0(void);
void func_8001E148(void);
void func_8001E1B8(void);

s32 func_80014874(TaskCallback callback);
s32 func_800148B4(TaskCallback callback);
s16 func_80014930(GameTask *parent, TaskCallback callback);
void func_800149A8(void);
void func_800149E0(void);
void setCurrentTaskCallback(TaskCallback callback);
void resetTaskCallbackOnFlag4(void);
void restoreSavedStateAndHandleFlag4(void);
void func_800719A8(s16 mode);
void deleteCurrentTask(void);
u32 allocateTaskResource(void);
void releaseTaskResource(u16 resource);

void setTaskSecondaryResourceData(GameTask *task, const u16 *data);

s32 isTaskResourceTransitionComplete(s16 index);
void clearControllerFlag5(u16 controller);
s16 hasTaskResource16Value(GameTask *task);

s32 hasTaskStateValue6(s32 id);
void func_80070274(s32 mode, s32 id, u16 duration);
s32 func_800707F0(s32 id);
void func_80071490(s32 mode, s32 value1, s32 value2);

void createMovingLineTask(s32 index);
void startIndexedRenderTask(s32 index);
void startIndexedRenderTaskPair(s32 index);
void initializeMode1Task(void);
void resetTaskControlState(void);
void initializeTaskCoordinateResource(void);
void initializePlayerCoordinateEffectTask(void);
void initializePlayerDualResourceEffectTask(void);
void initializeDualResource4C80EffectTask(void);
void initializeDualResource5100EffectTask(void);
void initializeSlidingVisualSequenceTask(void);
void initializeDelayedPlayerRiseEffectTask(void);
void initializeDualResourceSequenceTask(void);
void initializeModeDependentDualResourceTask(void);
void initializeScaledTaskCoordinateResource(void);
GameTask *createCountdownTask(u32 frames);
GameTask *createTimedSpriteB5Task(s16 x, s16 y, u32 frames);
GameTask *createValueTransitionTask(u32 *value, u32 target, u32 step);
GameTask *createAnimatedSpriteTask(s16 x, s16 y, s16 unk02, s16 repeats, u16 depth);
GameTask *createShrinkingSpriteTask(s16 motionEnabled, u16 spriteIndex);


void begin30TickTransitionOnFlag10(void);

s16 getTaskResourceGroupedIndex(GameTask *task, s16 index);
s16 getTaskResourceAdjustedGroupedIndex(s16 value, GameTask *task, s16 index);

void copyIndexedTaskImagesToVram(s16 index);

void finishMusicTaskAtDeadline(void);

void resetStateAndBeginResourceLoadTransition(u8 requestLoad);

#if defined(TASK_FLAGS_ABSOLUTE_ADDRESSING)
extern u8 D_8013AFD7 __attribute__((section(".data")));
#else
extern u8 D_8013AFD7;
#endif

void func_8001DB7C(void);
void initializeViewAndSelectTaskCallback(void);

void initializeGameOptionViewTask(void);

void initializeDualResourceTaskAndNotifyParent(void);

void initializeMainTailDualResourceTask(void);

void initializeModeDependentDualResourceTask(void);

void initializePlayerVisualPairTask(void);
void initializePlayerVisualPairTask5E80(void);
void initializePlayerVisualPairTask6780(void);

void initializeFlagSelectedDualResourceTask(void);

void beginSound601TaskSequence(void);
