#pragma once

#include "decomp/types.h"

typedef void (*TaskCallback)(void);

extern TaskCallback D_8013AFC8;
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
    /* Task-specific indices into gTaskMemoryPool. */
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

/* One set bit for each free block in gTaskMemoryPool. */
extern u16 gTaskMemoryFreeBits[16];

/* Scheduler storage remains in its original data object. */
/* Some task flows address this pointer absolutely; the scheduler uses $gp. */
extern GameTask *gCurrentTask;
extern GameTask *gCreatedTask;
extern GameTask *gCurrentTaskParent;
extern GameTask *D_8013B304;
/* GP-relative alias for the player snapshot task pointer. */
extern GameTask *playerSnapshotTaskGp;
/* Task transition flags consumed by the main presentation flow. */
extern u32 gTaskTransitionFlags;
extern u32 taskTransitionFlagsAbsolute __attribute__((section(".data")));
extern GameTask gGameTasks[200];
/* Each pool index selects a 256-byte block interpreted by the task using it. */
typedef struct TaskMemoryBlock {
    u32 unk00;
    u32 unk04;
    const u16 *unk08;
    u8 unk0C[0xF4];
} TaskMemoryBlock;

extern TaskMemoryBlock gTaskMemoryPool[256];

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

/* Creates a child of the current task. On exhaustion, the free-list tail is the fallback in gCreatedTask. */
s32 createChildTask(TaskCallback callback);
/* Leaves ten free task slots in reserve and uses the same fallback as createChildTask. */
s32 createChildTaskWithReserve(TaskCallback callback);
s32 tryCreateChildTaskWith20SlotsReserved(TaskCallback callback);
s16 createChildTaskForParent(GameTask *parent, TaskCallback callback);
s16 createChildTaskForParentWithReserve(GameTask *parent, TaskCallback callback);
s16 tryCreateChildTaskForParentWith20SlotsReserved(GameTask *parent, TaskCallback callback);
void deleteCurrentTaskSubtree(void);
void deleteCurrentTaskDescendants(void);
void deletePendingTaskSubtree(void);
u16 currentTaskHasChildren(void);
void setCurrentTaskCallback(TaskCallback callback);
void resetTaskCallbackOnFlag4(void);
void restoreSavedStateAndHandleFlag4(void);
void setSecondaryDisplayTaskMode(s16 mode);
void deleteCurrentTaskWhenCdCommandQueueIdle(void);
void deleteCurrentTask(void);
u32 allocateTaskMemoryBlock(void);
void freeTaskMemoryBlock(u16 blockIndex);

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

/* One-based attract-mode race selection; zero selects normal play. */
extern u8 gDemoRaceIndex;

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

extern u8 gSelectionMenuFadeStarted __attribute__((section(".data")));
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

struct MenuNavigationSound;
void createRegisteredSpacedChildTasks(s16 controllers, u32 acceptMask, u32 cancelMask,
                                      const u8 *itemData, const u8 *mapping,
                                      const struct MenuNavigationSound *sounds, u8 arg6, s8 wrapSelection, u8 blinkFrames,
                                      s16 x, s16 y, s16 spacingX, s16 spacingY,
                                      s16 count, s16 markerSprite, TaskCallback callback3C,
                                      TaskCallback callback40, s32 value, s32 index);
void initializeSpacedChildTasks(void);

void initializeDemoPlaybackTask(void);

typedef char GameTaskSizeCheck[(sizeof(GameTask) == 0x20) ? 1 : -1];
typedef char TaskMemoryBlockSizeCheck[(sizeof(TaskMemoryBlock) == 0x100) ? 1 : -1];
