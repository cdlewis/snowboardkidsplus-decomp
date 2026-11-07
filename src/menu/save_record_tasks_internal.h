#pragma once

/* Original unit addressing policy; include before public headers. */
#include "common.h"
#include "game/save/save_data.h"
#include "game/graphics/graphics.h"
#include "game/engine/game_options.h"
#include "game/engine/linear_random.h"
#include "game/engine/offset_archive.h"
#include "game/audio/music.h"
#include "psx/libsnd.h"
#define TASK_ABSOLUTE_ADDRESSING
#include "game/engine/task.h"
#include "game/race/player/race_player.h"
#include "game/menu/movement.h"
#include "game/menu/menu_selection_state.h"
#include "game/race/player/player_resource_textures.h"

typedef struct PaletteRotationResource {
    RECT rect;
    u16 original[16];
    u16 rotated[16];
    s16 interval;
    s16 timer;
    s8 first;
    s8 last;
    s8 steps;
} PaletteRotationResource;

typedef struct PaletteEntryResource {
    RECT rect;
    u16 original[16];
    u16 modified[16];
    s8 destination;
    s8 source;
} PaletteEntryResource;

typedef struct PaletteRotationSlot {
    GameTask *task;
    s16 state;
    s16 unk06;
} PaletteRotationSlot;

/* Selection bytes at player offsets 8..12, with the full player stride. */
typedef struct PlayerSelectionRecord {
    GameTask *task;
    u16 playerIndex;
    u8 unk06[2];
    u8 selectionBytes[5];
    u8 unk0D[0x9CF];
} PlayerSelectionRecord;

typedef struct RegisteredPanelSlot {
    GameTask *task;
    s16 command;
    s16 flags;
} RegisteredPanelSlot;

typedef struct GroupedIndexResource {
    u8 unk00[0x48];
    const u8 *values;
    u8 unk4C[0xB4];
} GroupedIndexResource;

typedef struct RegisteredTripletData {
    ByteTriplet *records;
    u8 *indices;
    s16 offsetX;
    s16 offsetY;
} RegisteredTripletData;

typedef struct HalfwordPair {
    s16 unk_00;
    s16 unk_02;
} HalfwordPair;

typedef struct IndexedHalfwordRecord {
    s16 value;
    s16 unk_02;
    s16 unk_04;
} IndexedHalfwordRecord;

typedef struct SelectionTransitionEntry {
    s16 selection;
    s16 previousSelection;
    s16 timer;
} SelectionTransitionEntry;

typedef struct StateValueHistory {
    s32 unk_00;
    s16 activeIndex;
    s16 current[2];
    s16 previous[2];
    s16 unk_0E[2];
    s16 unk_12[2];
    s16 unk_16[2];
} StateValueHistory;

extern u16 litObjectDrawAreaDepths[4] __attribute__((section(".data.lit_object_draw_area_depths")));

extern HalfwordPair D_8012F13C[16] __attribute__((section(".data")));

extern u16 gTaskShadowedText[14] __attribute__((section(".data.task_shadowed_text")));

extern StateValueHistory D_8013BE30 __attribute__((section(".bss.state_value_history")));
extern u8 D_8013AFD8 __attribute__((section(".data")));
extern u8 D_8013AFD9 __attribute__((section(".data")));
extern u8 D_8013B47C;
extern u8 D_8013B6B8[1];
extern u8 D_8013B6B9[7];

void func_800897D0(SaveRecord *record);
void mergeSaveRecordFlags(SaveRecord *source, SaveRecord *destination);
void func_800875D0(void);
void func_8007D5C8(void);
void updatePaletteRotationTask(void);
void func_80089B30(void);
void func_80087030(void);
void func_800857C4(void);
void func_800843DC(void);
void func_80084970(void);
void func_800801A0(void);
void func_800815D0(void);
void initializeDualResourceTaskAndResetByteState(void);
void adjustGroupedIndexForMode(s16 mode, u8 *index);
void func_8007F630(void);
void func_80081DF0(void);
void func_800816D8(void);

void initializeModeDependentDualResourceTask(void);
void createPaletteRotationTask(s32 x, s32 y, s32 first, s32 last, s32 steps, s32 interval, s32 index);
void clearHalfwordPairTable(void);
void initializeCountedPlayerGroupedSelectionBoundsTask(void);
void initializeDualResourceTaskAndClearState(void);
s32 getPlayerRecordFlagMode(s16 index);
s32 getSaveRecordFlagGroupIndex(s16 recordIndex, s16 groupIndex);
s16 getTaskResourceGroupedIndex(GameTask *task, s16 index);
s16 getTaskResourceAdjustedGroupedIndex(s16 value, GameTask *task, s16 index);
s16 getGroupedIndexBase(s16 index);
s16 getGroupedIndexOffset(s16 index);
s16 getGroupedIndexOffsetPlus216(s16 index);
s16 findIndexedHalfwordRecord(IndexedHalfwordRecord *records, u8 *indices, s16 count, s16 value);
void createRegisteredPanelTask(s32 mode, s32 value, s16 x, s16 y, s16 depth, s32 index);
void createRegisteredTripletTask(s32 x, s32 y, s32 offsetX, s32 offsetY, ByteTriplet *records, u8 *indices, s32 depth, s32 index);
void sortLitObjectWithDrawArea(GsDOBJ2 *object, u16 index, RECT *bounds);
ByteTriplet *lookupByteTriplet(ByteTriplet *records, u8 *indices, s16 index);
void updateRegisteredChildBlinkTransition(void);
void drawTaskOffsetSpriteB8(void);
void createPlayerGroupedSelectionTask(s32 player, s32 index);
void initializeDualResourceTask(void);
s16 mapGroupedValueToTableIndex(s16 value);
void updatePaletteEntryTask(void);
s32 isValue16(s32 value);
s16 hasStateValueChanged(s16 index);
void startClearedStateValueHistoryTask(void);
void initializeSaveRecordTask(void);
void resetUnlockNotificationStateForMode2(void);
void repeatByteStore(u8 *destination, s32 value, s32 count);
void copyGlobalFlagState(GlobalFlagStatePrefix *source, GlobalFlagStatePrefix *destination);
void copySaveRecordAndMergeFlags(void);
u32 calculateByteSum(const u8 *data, u32 size);
void startStateValueHistoryTask(void);
void drawTaskShadowedText(void);
s16 getSaveRecordMaskTier(s16 recordIndex, s16 maskIndex);
void setSaveRecordMaskTier(s16 recordIndex, s16 maskIndex, s16 tier);
s16 getStateMaskTier(s16 stateIndex, s16 maskIndex);
s16 getSaveRecordFlagPair(s16 recordIndex, s16 maskIndex, s16 pairIndex);
void setSaveRecordFlagPair(s16 recordIndex, s16 maskIndex, s16 pairIndex, s16 value);
s32 getStateMaskPair(s16 stateIndex, s16 maskIndex, s16 pairIndex);
void rebuildCombinedStateMask(GlobalFlagStatePrefix *state);
u32 getGlobalFlagMask(s16 bitIndex);
void updatePairedStateFlag(s16 index);
void setPairedStateIndexedFlag(s16 index);
s32 getModeFilteredGameOptionFlag2(void);
s32 getStateAdjustedGameOptionFlag1(void);
