#pragma once

#include "common.h"
#include "game/save/save_data.h"
#include "game/graphics/graphics.h"
#include "game/engine/game_options.h"
#include "game/engine/linear_random.h"
#include "game/engine/offset_archive.h"
#include "game/audio/music.h"
#include "psx/libsnd.h"
#include "game/engine/task.h"
#include "game/engine/system_runtime.h"
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
extern TexturedQuadListEntry taskIndexPanelQuadsPalette0[10];
extern TexturedQuadListEntry taskIndexPanelQuadsPalette1[10];

void func_800897D0(SaveRecord *record);
void mergeSaveRecordFlags(SaveRecord *source, SaveRecord *destination);
void updateSaveRecordTask(void);
void updateSelectionPrizeMenuTask(void);
void updatePaletteRotationTask(void);
void updateStateValueHistoryTask(void);
void updateMemoryCardSlotTask(void);
void func_800857C4(void);
void updatePlayerGroupedSelectionMenu(void);
void updatePlayerGroupedSelectionTask(void);
void updateRecordFlagSelectionMenuTask(void);
void func_800815D0(void);
void initializeDualResourceTaskAndResetByteState(void);
void updatePlayerGroupedCourseSelectionTask(void);
void updateRegisteredTripletTask(void);
void updateRegisteredPlayerModels(void);

void clearHalfwordPairTable(void);
void initializeDualResourceTaskAndClearState(void);
void sortLitObjectWithDrawArea(GsDOBJ2 *object, u16 index, RECT *bounds);
ByteTriplet *lookupByteTriplet(ByteTriplet *records, u8 *indices, s16 index);
void updateRegisteredChildBlinkTransition(void);
void drawTaskOffsetSpriteB8(void);
void drawTaskIndexPanel(void);
void drawSaveRecordProgressPanel(void);
void initializeDualResourceTask(void);
void updatePaletteEntryTask(void);
s32 isValue16(s32 value);
s16 hasStateValueChanged(s16 index);
void startClearedStateValueHistoryTask(void);
void resetUnlockNotificationStateForMode2(void);
void repeatByteStore(u8 *destination, s32 value, s32 count);
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
u32 getGlobalFlagMask(s16 bitIndex);
void updatePairedStateFlag(s16 index);
void setPairedStateIndexedFlag(s16 index);
s32 getModeFilteredGameOptionFlag2(void);
s32 getStateAdjustedGameOptionFlag1(void);

void createRegisteredPlayerModelTask(s16 slot, s16 selection, s16 modelIndex,
                                     s16 x, s16 y, s16 z, s32 index);
void reloadTaskPlayerModel(GameTask *task, s16 slot, s16 selection, s16 modelIndex, s16 x, s16 y, s16 z);
