#pragma once

/* Original unit addressing policy; include before public headers. */
#define RACE_FINISH_ABSOLUTE_ADDRESSING
#include "common.h"
#include "game/text/text_layout.h"
#include "game/scene/scene_sound_slots.h"
#include "game/io/controller.h"
#include "game/io/controller_effects.h"
#include "game/save/save_data.h"
#define SELECTED_RECORDS_ABSOLUTE_ADDRESSING
#include "game/menu/selected_records.h"
#include "game/io/cd_queue.h"
#define GAME_OPTION_TRANSITION_ABSOLUTE_ADDRESSING
#include "game/engine/game_options.h"
#include "game/graphics/graphics.h"
#define MUSIC_STATE_ABSOLUTE_ADDRESSING
#define MUSIC_ABSOLUTE_ADDRESSING
#include "game/audio/music.h"
#include "psx/libsnd.h"
#include "psx/libsn.h"
#include "psx/libc.h"
#define RACE_PLAYER_ABSOLUTE_ADDRESSING
#define TASK_ABSOLUTE_ADDRESSING
#define TASK_FLAGS_ABSOLUTE_ADDRESSING
#include "game/race/player/race_player.h"
#include "game/scene/scene_events.h"
#include "game/engine/task.h"
#include "game/menu/selected_sprite.h"
#include "game/menu/movement.h"
#include "psx/libgpu.h"
#include "psx/libgs.h"
#include "psx/libsn.h"
#include "psx/libc.h"
#include "game/scene/scene_task_types.h"
#include "game/scene/scene_audio_types.h"
#include "game/menu/number_selection_types.h"
#include "game/graphics/draw_mode_sprite.h"

#define DRAW_MODE_COMMAND_HIGH 0xE100
#define TEXTURED_SPRITE_CODE 0x64

typedef struct SelectionMenuState {
    s16 selection, previousSelection;
    s16 unk04, unk06, unk08, unk0A;
    u8 unk0C[0xF4];
} SelectionMenuState;

#include "unlock_menu_types.h"


typedef struct ControllerPlaybackEntry {
    u8 course;
    u8 pad;
    s16 sector;
    s16 count;
} ControllerPlaybackEntry;


extern ControllerPlaybackEntry gControllerPlaybackEntries[12] __attribute__((section(".data.controller_playback_entries")));

extern s16 gControllerPlaybackDurations[12] __attribute__((section(".data.controller_playback_durations")));

extern u8 D_8013AF3A __attribute__((section(".data")));
extern u16 D_8013AFAA __attribute__((section(".data")));
extern s16 D_8013B10E __attribute__((section(".data")));
extern s16 D_8013B10C __attribute__((section(".data")));
extern u8 D_8013B55E __attribute__((section(".data")));
/* This transition uses $gp; the playback transition below uses absolute access. */
extern u8 modeTransitionReadyGp;

extern u8 D_8013B564;

extern s16 D_8013B560;
extern s16 selectionMenuRecordAbsolute __attribute__((section(".data")));
extern u8 D_8013B562;
/* Playback cleanup addresses this shared transition byte absolutely. */
extern u8 playbackTransitionByteAbsolute __attribute__((section(".data")));
extern char D_80012310[];
extern char D_8001232C[];
extern char D_8013B5C8[];

void func_800187B8(u8 voice);
void func_8001814C(void);
int printf(const char *format, ...);

void func_80040D5C(void);
void initializeNumberSelectionTask(void);
void completeModeSelectionTransitionWhenReady(void);
void func_800A2D68(void);
void finishSelectionMenuTask(void);
void func_800A31C0(SelectionMenuState *state);
void initializeModeSectorReadResourcesTask(void);
void func_800A24EC(void);
void selectTaskBySelectedRecordMatch(void);
void SetFogNearFar(s32 near, s32 far, s32 h);
void func_800A2C40(void);
void func_800A25F8(void);
void initializeControllerPlaybackRaceTask(void);
void initializeRaceTaskState(void);

void finishControllerPlaybackTransition(void);
void func_8002FA58(s16 resource);
void func_800A42CC(void);
void func_800A499C(void);
void func_8001B858(void);
void func_800A1F50(void);
void func_800A4C28(void);
void func_800A3D9C(void);
void func_800A525C(void);

typedef struct NumberPanelState {
    u16 unk00[5];
    u16 number;
} NumberPanelState;

void func_800A53BC(void);
void finishNumberAdjustmentTask(void);
void func_8002F394(void);
void func_800A5630(void);
void beginMode2SequenceAndFadeMusic(void);
void startUnlockResourceLoad(void);
void prepareUnlockSelectionResume(void);

void advanceTaskAtModeCoursePosition(void);
void startMode1TaskTransition(void);
void drawNumberSelectionPanel(NumberPanelState *state);
void drawOffsetSelectedTaskSprite(void);
void initializeSelectionMenuTask(void);
void advanceTaskWhenFlagB55ESet(void);
void updateControllerPlaybackUntilFade(void);
void initializeUnlockMenuResourceTask(void);
void runTaskResource14Update(void);
void finishUnlockMenuTask(void);
void initializeUnlockMenuTask(void);
s32 hasUnlockNotificationFlag(s16 index);
void startUnlockActionAfterCountdown(void);
void animateUnlockMenuTransition(void);
void resumeUnlockSelectionTask(void);
void initializeNumberAdjustmentTask(void);
void updateNumberAdjustmentTask(void);
void drawNumberAdjustmentArrow(s16 direction, GsOT *ot, u16 depth);
void func_800A5AD4(void);
void func_800A5ADC(void);
void func_800A5AE4(void);
void func_800A5AEC(void);
void func_800A5AF4(void);
void playMusicSequence(s16 count, u8 left, u8 right);
