#pragma once

/* Original unit addressing policy; include before public headers. */
#include "common.h"
#include "game/text/text_sound_slots.h"
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
#include "game/text/text_entries.h"
#include "game/engine/task.h"
#include "game/menu/selected_sprite.h"
#include "game/menu/movement.h"
#include "psx/libgpu.h"
#include "psx/libgs.h"
#include "psx/libsn.h"

#include "../race/race_internal.h"
#include "game/text/text_task_types.h"
#include "game/audio/text_audio_types.h"
#include "game/graphics/draw_mode_sprite.h"

#define DRAW_MODE_COMMAND_HIGH 0xE100
#define TEXTURED_SPRITE_CODE 0x64

typedef struct SecondaryOwner {
    u8 unk00[0x24];
    u32 tick;
    u16 flags;
    u16 resource;
    GameTask *task;
    u8 unk30[0x10];
    u32 completionTick;
} SecondaryOwner;

typedef struct AudioTransitionState {
    u8 unk00[0x2C];
    s32 unk2C[5];
} AudioTransitionState;

extern s16 gTextEntryCdCommands[230] __attribute__((section(".data.text_entry_cd_commands")));

extern CdResourceLocation gSecondaryResourceLocations[108] __attribute__((section(".data.secondary_resource_tables")));
extern u8 gSecondaryResourceModeOffsets[12] __attribute__((section(".data.secondary_resource_tables")));

extern GameTask *gSecondaryResourceDisplayTask __attribute__((section(".sdata.secondary_resource_display_task")));

extern u8 D_8013AF3A __attribute__((section(".data")));
extern char D_80012310[];
extern char D_8001232C[];
extern char D_8013B5C8[];
extern u16 gTextOneShotSoundKey;
extern u8 gTextOneShotVoice;

void func_800187B8(u8 voice);
void func_8001814C(void);
void func_800A6E88(void);
void updateTextEntryCdAudioTask(void);
void signalFlag9WhenTaskCountdownExpires(void);
int printf(const char *format, ...);

void setSecondaryResourceEntryTask(TextEntry *entry);
void initializeSecondaryResourceTask(void);
void updateSecondaryResourceTask(void);
void drawSecondaryTextBackgroundTask(void);
void setSecondaryTextEntryTask(TextEntry *entry);
void setTextEntryMusicTask(TextEntry *entry);
void finishMusicTaskAtDeadline(void);
void setTextEntryCdAudioTask(TextEntry *entry);
void processTextEntrySound(TextEntry *entry);
void stopTextSoundVoices(void);
void beginAudioFadeCountdownTask(void);
void startUnlockNotification(s16 index);
void drawPagedTexturedRectangle(s16 x, s16 y, s16 width, s16 height, s16 tpage,
                   s16 clut, GsOT *ot, u16 depth);
int readHostFile(char *path, char *buffer);
int writePcHostFile(char *name, char *buffer, int length);
s16 readPcHostAsset(s16 mode, char *name, void *buffer, s16 *position);
