#pragma once

/* Original unit addressing policy; include before public headers. */
#include "common.h"
#include "game/scene/scene_sound_slots.h"
#include "game/io/controller.h"
#include "game/io/controller_effects.h"
#include "game/save/save_data.h"
#define RACE_FINISH_ABSOLUTE_ADDRESSING
#define SELECTED_RECORDS_ABSOLUTE_ADDRESSING
#include "game/menu/selected_records.h"
#include "game/io/cd_queue.h"
#include "game/engine/byte_search.h"
#include "game/engine/game_options.h"
#define GAME_LOOP_ABSOLUTE_ADDRESSING
#include "game/engine/game_loop.h"
#include "game/graphics/graphics.h"
#define MUSIC_ABSOLUTE_ADDRESSING
#include "game/audio/music.h"
#include "psx/libsnd.h"
#define RACE_PLAYER_ABSOLUTE_ADDRESSING
#define TASK_MODE_STATE_ABSOLUTE_ADDRESSING
#define TASK_ABSOLUTE_ADDRESSING
#define TASK_FLAGS_ABSOLUTE_ADDRESSING
#include "game/race/player/race_player.h"
#include "game/race/camera/race_camera.h"
#include "game/graphics/coordinate.h"
#include "game/scene/scene_events.h"
#include "game/engine/task.h"
#include "game/menu/movement.h"
#include "psx/libgpu.h"
#include "psx/libgs.h"
#include "game/menu/menu_selection_state.h"
#include "game/graphics/coordinate.h"
#include "game/engine/linear_random.h"
#include "game/scene/scene_task_types.h"
#include "game/scene/scene_audio_types.h"
#include "game/menu/number_selection_types.h"

#define INDEXED_SECTOR_READ_DESTINATION DAT_INDEXED_RESOURCE

typedef struct PaletteBurstResource {
    s16 x, y, z, unk06;
    s16 rotation, unk0A;
    s32 age;
    u8 r, g, b;
} PaletteBurstResource;
typedef struct ModelPreviewState {
    s16 unk00;
    u16 rotationStep;
} ModelPreviewState;

typedef struct SelectionDisplayEntry {
    s16 index;
    s16 unk02;
    s16 unk04;
} SelectionDisplayEntry;

typedef struct GridMarkerState {
    u8 unk00[0x30];
    u16 state;
    s16 selectedIndex;
    u8 unk34[0x1C];
    u16 depth;
    u8 unk52[0x10];
    u8 red, green, blue;
} GridMarkerState;

typedef struct RegisteredDetailedPanelResource {
    s16 unk00, unk02, unk04, unk06, unk08;
    s16 x, y, textX, textY;
    const u16 *text;
    s16 value, color, mode;
    const u16 *content;
    s32 depth, index;
} RegisteredDetailedPanelResource;

/* Menu descriptors have a 0x14-byte stride in the callers' tables. */
typedef struct ByteMappingDescriptor {
    const u8 *mapping;
    u8 unk04[2];
    u8 count;
    u8 unk07[13];
} ByteMappingDescriptor;

typedef struct ModeSectorReadState {
    u8 unk00[0xA];
    s16 sectorIndex;
    u8 unk0C[0x2C];
    s16 mode;
    u8 unk3A[0xC6];
} ModeSectorReadState;
typedef struct TextSequenceResource {
    u8 unk00[0xA];
    u16 unk0A;
    u16 entryIndex;
    u8 unk0E[6];
    s32 x, y, velocity;
    u8 unk20[0xC];
    s32 displacement;
    u16 progress;
    u16 duration;
    s16 state;
    u8 intensity;
    u8 unk37;
    s16 mode;
    u16 unk3A;
    u8 unk3C[0xC4];
} TextSequenceResource;

typedef struct SplitBoundsResource {
    u8 unk00[0x1C];
    s16 unk1C;
    u8 unk1E[6];
    s16 unk24;
} SplitBoundsResource;

typedef struct SplitSelectionBoundsConfigs {
    u8 entries[10][3];
    u8 padding[2];
} SplitSelectionBoundsConfigs;

#include "unlock_menu_types.h"

extern s16 D_8013B10C __attribute__((section(".data")));
extern u8 D_8013B55E;
extern u8 D_8013B527 __attribute__((section(".data")));

extern s8 D_80011DC8[];

extern u8 gBoardSelectionLevels[12] __attribute__((section(".data.board_selection_levels")));
extern s8 *gBoardStyleText[4] __attribute__((section(".data.board_style_text")));
extern u8 gBoardSelectionStyles[12] __attribute__((section(".data.board_selection_styles")));
extern const u8 gBoardLevelLabel[16] __attribute__((section(".rodata.board_level_label"), aligned(4)));

extern u16 gCurrentTaskPanelText[8] __attribute__((section(".data.current_task_panel_text")));

extern const u16 gTaskSelectionPanelLabel[6] __attribute__((section(".data.task_selection_panel_text"), aligned(4)));
extern const u16 gTaskSelectionPanelText[8] __attribute__((section(".data.task_selection_panel_text"), aligned(4)));

extern const u16 gTaskPanel144x48Text[8] __attribute__((section(".data.task_panel_144x48_text")));

extern const u16 gTaskPlayerValuePanelLabel[6] __attribute__((section(".data.task_player_value_panel_label")));
extern const u16 gTaskPlayerValuePanelText[8] __attribute__((section(".data.task_player_value_panel_text")));

extern u32 gPlayerPaletteTransitionPixels[8] __attribute__((section(".data.player_palette_transition_pixels")));
extern ResourceSectorRange gGridResourceSecondSectorRanges[10][5] __attribute__((section(".data.grid_resource_sector_ranges")));

extern ResourceSectorRange gGridResourceFirstSectorRanges[10][5] __attribute__((section(".data.grid_resource_sector_ranges")));

extern ResourceSectorRange gMode1SectorRanges[10] __attribute__((section(".data.indexed_sector_ranges")));
extern ResourceSectorRange gOtherModeSectorRanges[11] __attribute__((section(".data.indexed_sector_ranges")));

extern POLY_F3 gNumberAdjustmentArrow __attribute__((section(".data.number_adjustment_arrow")));

extern SplitSelectionBoundsConfigs gSplitSelectionBoundsConfigs __attribute__((section(".data.split_selection_bounds_configs")));

extern u16 playerModelDrawAreaDepths[4] __attribute__((section(".data.player_model_draw_area_depths")));

extern u8 D_8013B514[3] __attribute__((section(".sdata.four_bounds_task_flag")));
extern u8 D_8013B517 __attribute__((section(".sdata.four_bounds_task_flag")));

extern s16 D_8013B560;
extern u8 D_8013B562;
extern u8 D_8013AF3A __attribute__((section(".data")));

void func_8009488C(void);
void func_800187B8(u8 voice);

void func_8008CD98(void);
void func_8008FA8C(void);
void func_8008D200(void);
void func_8008DDC0(void);
void func_8008EC50(void);
void func_80090450(void);
void func_80092090(void);
void func_80097FF4(void);
void func_80099880(void);
void func_800A0FDC(void);
void func_800A1774(TextSequenceResource *resource);
void func_80092A50(void);
void prepareRaceResourcesIfSelectionChanged(void);
void SetFogNearFar(s32 near, s32 far, s32 h);
void func_800A0EE8(void);
void queueTaskModeSectorRead(void);
void queueModeIndexedSectorRead(s16 mode, s16 index);
void initializeControllerPlaybackRaceTask(void);
void func_8009B700(void);
void func_8009BDB8(void);
void func_8009CA90(void);
void func_8009F9B8(void);
void func_8009F224(void);

void func_8002FA58(s16 resource);
void func_800A42CC(void);
void func_800A499C(void);
void preparePlayerResourcesTask(void);
void initializeRaceTaskState(void);
void func_8001B858(void);
void func_800A1F50(void);
void func_800A4C28(void);
void func_800A525C(void);
void func_800A53BC(void);
void func_8009D354(void);
void func_800A05C0(void);
void startUnlockResourceLoad(void);
void prepareUnlockSelectionResume(void);
void func_8003D940(void);
void func_8003EE70(void);
void updateTaskViewAndCheckPlayerLimit(void);

void createRegisteredDetailedTextPanelTask(s16 x, s16 y, s16 textX, s16 textY,
                                   const u16 *text, s32 value, s32 color, s32 mode,
                                   const u16 *content, s32 depth, s32 index);
void initializeCountedTaskResourcePair(void);
void initializeAlternateCountedTaskResourcePair(void);
void initializeCountedTaskResourcePairEC50(void);
void initializeCountedGroupedSelectionBoundsTask(void);
void initializeTaskResourcePair(void);
void initializeCountedSelectionBoundsTask(void);
void initializeTaskResourcesAndClearData18(void);
void drawCurrentTaskBoardStyleAndLevel(void);
void drawCurrentTaskSpriteBA(void);
void drawCurrentTaskFramedSpritePair16A(void);
void initializeCountedDisplayBoundsTask(void);
s32 hasStateGridFlag(GlobalFlagStatePrefix *state, s16 row, s16 column);
void setStateGridFlag(GlobalFlagStatePrefix *state, s16 row, s16 column);
void drawCurrentTaskFramedSprite20C(void);
void drawCurrentTaskTextPanel(void);
void drawCurrentTaskSelectionPanel(void);
void queueGridResourceSectorReads(s16 slot, s16 row, s16 column);
void sortPlayerModelWithDrawArea(u16 playerIndex, u16 depthIndex, RECT *bounds);
void loadCurrentPlayerModelPose(const s16 *values);
void setCurrentPlayerFinalPartTranslations(const s16 *values);
void updateCurrentPlayerModelPartCoordinates(void);
void updatePlayerModelPreviewTask(void);
void initializeCountedSplitSelectionBoundsTask(void);
void updateSplitSelectionBoundsTransition(void);
void drawCurrentTaskSpriteB7(void);
void drawCurrentTaskFramedSprite20B(void);
void drawCurrentTaskTextPanel144x48(void);
void drawCurrentTaskPlayerValuePanel(void);
void initializeCountedFourBoundsTask(void);
void initializeTaskGridPositions(s32 unused, GameTask *task, s32 x, s32 y);
void configureFourColumnTaskGridMovement(s32 mode, GameTask *task, s32 x, s32 y, s32 stepX, s32 stepY, s32 delay);
void drawTaskGridSelectionMarker(void);
void copyPlayerPaletteRowWithTransition(s16 playerIndex, s32 unused, s32 paletteRow, u16 mode);
void copyPlayerPaletteRow(s16 playerIndex);
void spawnPaletteBurstTask(u8 paletteIndex);
void initializeMainTailDualResourceTask(void);
void initializeSelectionIndexedDualResourceTask(void);
void initializeModeDependentTaskResourcePair(void);
void initializeFlagSelectedDualResourceTask(void);
void initializeModeDependentTailResourcePairTask(void);
s16 findIdentityMappedIndexOrZero(ByteMappingDescriptor *descriptor, s16 value);
void initializeTaskResourcePairWithOneTickTransition(void);
void selectTaskCallbackFromModeState(void);
void initializeMode1Task(void);
void initializeMode1RacePlayers(void);
void initializeModeSectorReadResourcesTask(void);
void initializeTextSequenceTask(void);
void animateTextSequenceExitTask(void);
void advanceModeAfterTextSequenceFadeTask(void);
void initializeTaskPlayerCamera(void);
