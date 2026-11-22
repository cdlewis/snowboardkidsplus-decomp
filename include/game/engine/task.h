#pragma once

#include "decomp/types.h"

typedef void (*TaskCallback)(void);

extern TaskCallback D_8013AFC8;
u16 func_80014B14(void);
void dispatchGameOptionTransitionTask(void);
void beginMode2SequenceAndFadeMusic(void);

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

/* Grid selection state in resource14; resource16 holds 8-byte entries. */
typedef struct TaskGridState {
    u8 unk00[0x32];
    s16 selectedIndex;
    u8 unk34[0x24];
    s16 spacingY;
    s16 count;
} TaskGridState;

typedef struct TaskGridEntry {
    GameTask *task;
    u32 unk04;
} TaskGridEntry;

typedef struct RegisteredTaskEntry {
    GameTask *task;
    s16 command;
    u16 flags;
} RegisteredTaskEntry;

typedef struct ByteTriplet {
    u8 bytes[3];
} ByteTriplet;

/* One set bit for each available task resource. */
extern u16 gTaskResourceFreeBits[16];

/* Scheduler storage remains in its original data object. */
/* Some task flows address this pointer absolutely; the scheduler uses $gp. */
extern GameTask *D_8013AE88;
extern GameTask *D_8013AE8C;
extern GameTask *D_8013AE98;
extern GameTask *D_8013B304;
/* GP-relative alias for the player snapshot task pointer. */
extern GameTask *playerSnapshotTaskGp;
/* Task transition flags consumed by the main presentation flow. */
extern u32 D_8013AFE4;
extern u32 taskTransitionFlagsAbsolute __attribute__((section(".data")));
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
void initializeCollectibleTask(void);
void initializePickupVisualTask(void);
void initializePlacedObjectTask(void);
void initializeOrbitingTaskObject(void);
void initializeVisualTrailTask(void);
void updateBobbingTrailEmitterTask(void);
/* Initial scheduler callbacks implemented by the main presentation flow. */
void armTaskCleanupOnFlagAFAABit6(void);
void armTaskCallbackResetOnFlag3(void);
void armSavedStateRestoreOnFlag3(void);

/* Creates a child of the current task. On exhaustion, the free-list tail is the fallback in D_8013AE8C. */
s32 createChildTask(TaskCallback callback);
/* Leaves ten free task slots in reserve and uses the same fallback as createChildTask. */
s32 createChildTaskWithReserve(TaskCallback callback);
s32 tryCreateChildTaskWith20SlotsReserved(TaskCallback callback);
s16 createChildTaskForParent(GameTask *parent, TaskCallback callback);
s16 createChildTaskForParentWithReserve(GameTask *parent, TaskCallback callback);
s16 func_8001496C(GameTask *parent, TaskCallback callback);
void deleteCurrentTaskSubtree(void);
void deleteCurrentTaskDescendants(void);
void deletePendingTaskSubtree(void);
void setCurrentTaskCallback(TaskCallback callback);
void resetTaskCallbackOnFlag4(void);
void restoreSavedStateAndHandleFlag4(void);
void setSecondaryDisplayTaskMode(s16 mode);
void deleteCurrentTaskWhenCdCommandQueueIdle(void);
void deleteCurrentTask(void);
u32 allocateTaskResource(void);
void releaseTaskResource(u16 resource);

void setTaskSecondaryResourceData(GameTask *task, const u16 *data);

s32 isTaskResourceTransitionComplete(s16 index);
void clearControllerFlag5(u16 controller);
s16 hasTaskResource16Value(GameTask *task);

s32 hasTaskStateValue6(s32 id);
void configureTaskStateTransition(s32 mode, s32 id, u16 duration);
s32 isTaskStateTransitionBelowMaximum(s32 id);
void setTaskBackgroundMode(s32 mode, s32 value1, s32 value2);

void createMovingLineTask(s32 index);
void createFlickeringDotsTask(s32 playerIndex);
void startIndexedRenderTask(s32 index);
void startIndexedRenderTaskPair(s32 index);
void createPlayerParticleTask(s32 playerIndex);
s32 createPlayerTripleQuadEffectTask(s32 playerIndex);
void initializeMode1Task(void);
void resetTaskControlState(void);
void initializeTaskCoordinateResource(void);
void initializePlayerCoordinateEffectTask(void);
void initializePlayerState23CoordinateEffectTask(void);
void initializePlayerDualResourceEffectTask(void);
void initializeDualResource4C80EffectTask(void);
void initializeDualResource5100EffectTask(void);
void initializeSlidingVisualSequenceTask(void);
void initializeDelayedPlayerRiseEffectTask(void);
void initializeOrbitingPlayerVisualTask(void);
void initializeFallingTaskObject(void);
void initializeDualResourceSequenceTask(void);
void initializeScaledTaskCoordinateResource(void);
GameTask *createCountdownTask(u32 frames);
GameTask *createTimedSpriteB5Task(s16 x, s16 y, u32 frames);
GameTask *createValueTransitionTask(u32 *value, u32 target, u32 step);
GameTask *createAnimatedSpriteTask(s16 x, s16 y, s16 unk02, s16 repeats, u16 depth);
GameTask *createShrinkingSpriteTask(s16 motionEnabled, s32 spriteIndex);

void begin30TickTransitionOnFlag10(void);

void copyIndexedTaskImagesToVram(s16 index);

void resetStateAndBeginResourceLoadTransition(u8 requestLoad);

extern u8 D_8013AFD7;

extern u8 D_8013AFD5;

void prepareRacePlayersAndQueueResources(void);
void initializeViewAndSelectTaskCallback(void);

void initializeGameOptionViewTask(void);

void initializeDualResourceTaskAndNotifyParent(void);
void updateMainMenuSelectionTask(void);

void initializeMainTailDualResourceTask(void);

void initializeModeDependentTailResourcePairTask(void);

void initializePlayerVisualPairTask(void);
void initializePlayerVisualPairTask5E80(void);
void initializePlayerVisualPairTask6780(void);

void initializeFlagSelectedDualResourceTask(void);

void beginSound601TaskSequence(void);

void initializeSelectionIndexedDualResourceTask(void);

void initializeProximityEmitterTask(void);

void initializeDelayedEightEntryEffectTask(void);

void initializeFlagSelectedCdAwareTask(void);
s32 func_8007D47C(void);
void updateCdAwareCourseSelectionTask(void);

void initializeSelectedCourseOffsetTask(void);
void updateSelectedCourseOffsetTask(void);

void initializeTaskResourcePairWithOneTickTransition(void);
void initializeSelectionMenuTask(void);
void initializeUnlockMenuResourceTask(void);

extern u8 D_8013B0F0;

void drawSelectedTaskSprite(void);

void initializeBouncingVisualTask(void);

void initializeExpandingCoordinateEffectTask(void);

void createPlayerPairedQuadEffectTask(s32 playerIndex, s32 variant);

void initializePlayerModeSelectionTask(void);

void configureTaskGridMovement(s32 mode, GameTask *task, s32 x, s32 y,
                               s32 stepX, s32 stepY, s32 delay);

void configureFourColumnTaskGridMovement(s32 mode, GameTask *task, s32 x, s32 y,
                                         s32 stepX, s32 stepY, s32 delay);

void configureVerticalTaskGridMovement(s32 mode, GameTask *task, s32 x, s32 y,
                                      s32 stepX, s32 stepY, s32 delay);

void initializeSpacedTaskGridPositions(s32 mode, GameTask *task, s32 x, s32 y,
                                 s32 spacingX, s32 spacingY);

extern u8 D_8013B41F __attribute__((section(".data")));
extern u8 selectionMenuFadeStartedGp;
void configureColorCycleTask(s32 mode, s16 minimum, s16 maximum, s32 step, s16 index);
void updateColorCycleTask(void);

void updateControllerPlaybackUntilFade(void);

void createRegisteredCallbackTask(s32 mode, s16 x, s16 y, TaskCallback callback,
                                  s32 value, s32 index);

void initializeMode1RacePlayers(void);
void initializeModeRaceTasks(void);
void queueRaceStartupResourcesTask(void);

void startUnlockNotification0Task(void);

void updateSelectedPlayerPlaybackTask(void);

void drawTaskGridSelectionMarker(void);

void createRegisteredSpacedChildTasks(s16 mode, u32 acceptMask, u32 cancelMask,
                                      const u8 *data44, const u8 *data48,
                                      const u8 *data4C, u8 arg6, s8 arg7, u8 arg8,
                                      s16 x, s16 y, s16 spacingX, s16 spacingY,
                                      s16 count, s16 arg14, TaskCallback callback3C,
                                      TaskCallback callback40, s32 value, s32 index);
void initializeSpacedChildTasks(void);

void initializeDemoPlaybackTask(void);

typedef char GameTaskSizeCheck[(sizeof(GameTask) == 0x20) ? 1 : -1];
typedef char TaskResourceRecordSizeCheck[(sizeof(TaskResourceRecord) == 0x100) ? 1 : -1];
