#pragma once

/* Original unit addressing policy; include before public headers. */
#include "common.h"
#define TASK_ABSOLUTE_ADDRESSING
#include "game/engine/task.h"
#include "game/graphics/graphics.h"
#include "game/graphics/coordinate.h"
#include "game/race/camera/race_camera.h"
#include "game/engine/linear_random.h"
#include "game/engine/offset_archive.h"
#include "game/audio/music.h"
#include "game/save/save_data.h"
#define RACE_PLAYER_ABSOLUTE_ADDRESSING
#include "game/io/cd_queue.h"
#include "game/io/controller.h"
#include "game/race/player/race_player.h"
#define GAME_OPTIONS_GP_ADDRESSING
#include "game/engine/game_options.h"
#define SELECTED_RECORDS_GP_ADDRESSING
#include "game/menu/selected_records.h"
#include "psx/libsnd.h"
#include "psx/libcd.h"
#include "psx/libgpu.h"
#include "psx/libgs.h"
#include "psx/libetc.h"
#include "psx/libmcrd.h"
#include "psx/libcd.h"
#include "psx/libpress.h"
#include "game/engine/system_runtime.h"

#define TMD_TEXTURE_HEADER(mode, flags, inputWords, outputWords) \
    (((mode) << 24) | ((flags) << 16) | ((inputWords) << 8) | (outputWords))

#define PLAYER_ANIMATION_ARCHIVE ((u8 *)0x80180000)
#define PLAYER_ANIMATION_OFFSETS ((u32 *)0x80180004)

typedef struct PaletteTransitionResource {
    u32 *pixels;
} PaletteTransitionResource;

typedef struct PaletteTransitionState {
    u16 *pixels;
    u16 red[16], green[16], blue[16];
    u16 redStep[16], greenStep[16], blueStep[16];
    u16 mask[16];
} PaletteTransitionState;
typedef struct PaletteRecordColor {
    u8 r, g, b, unk3;
} PaletteRecordColor;

typedef struct SoundVoiceRecord {
    u8 owner;
    u8 bank;
    u16 note;
} SoundVoiceRecord;

typedef struct SavedImageEntry {
    u32 flags;
    PaletteRecordColor color;
    RECT rect;
    u16 pixels[16];
    u16 transitionPixels[16];
} SavedImageEntry;

extern char D_800101B8[];
extern char D_800101D0[];
extern char D_800101E8[];
extern char D_80010200[];
extern char D_80010218[];

extern char *gMemoryCardFileNames[5] __attribute__((section(".data.memory_card_file_names")));

extern MemoryCardSaveTitle D_80010230;
extern MemoryCardSaveTitle D_80010274;
extern MemoryCardSaveTitle D_800102B8;
extern MemoryCardSaveTitle D_800102FC;
extern MemoryCardSaveTitle D_80010340;

extern MemoryCardSaveTitle *gMemoryCardSaveTitles[5] __attribute__((section(".data.memory_card_tables")));

extern u32 gMemoryCardSaveCapacities[5] __attribute__((section(".data.memory_card_tables")));

extern u8 D_80120200[16] __attribute__((section(".data.player_model_parts")));

extern u8 D_80120210[16] __attribute__((section(".data.player_model_parts")));

extern s16 D_80120220[42] __attribute__((section(".data.player_model_parts")));

extern s16 D_80120274[42] __attribute__((section(".data.player_model_parts")));

extern s16 D_801202C8[42] __attribute__((section(".data.player_model_parts")));

extern s16 D_8012031C[42] __attribute__((section(".data.player_model_parts")));

extern s16 D_80120370[42] __attribute__((section(".data.player_model_parts")));

extern s16 *D_801203C4[10] __attribute__((section(".data.player_model_parts")));
extern RacePlayerVector gPlayerAnimationPose[12] __attribute__((section(".data.player_animation_pose"), aligned(4)));

/* Nine bank flags with the original three trailing padding bytes. */
extern s8 gSoundBankLoaded[12] __attribute__((section(".data.sound_bank_loaded"), aligned(4)));

extern char gSoundSequenceTable[0xB0] __attribute__((section(".data.sound_sequence_table"), aligned(4)));

extern SoundVoiceRecord gSoundVoiceRecords[8] __attribute__((section(".data.sound_voice_records")));

extern MemoryCardSaveHeader gMemoryCardSaveHeader __attribute__((section(".data.memory_card_save_header")));

extern SavedImageEntry gSavedImageEntries[32] __attribute__((section(".data.saved_image_entries")));
extern u32 gSynchronizedFrameCount __attribute__((section(".sdata.synchronized_frame_count")));
extern s8 gSoundSystemActive;
extern u8 D_8013AF29;
extern s16 gMusicSequenceId;
extern u8 gMusicSequenceActive __attribute__((section(".sdata.music_state")));
extern u8 gMusicFadeMode __attribute__((section(".sdata.music_state")));
extern s16 gMusicFadeStep __attribute__((section(".sdata.music_state")));
extern s16 D_8013AF30 __attribute__((section(".sdata.music_state")));
extern s16 D_8013AF32 __attribute__((section(".sdata.music_state")));
extern s32 gLinearRandomState __attribute__((section(".data.linear_random_state")));
extern MovieDecodeState gMovieDecodeState __attribute__((section(".bss.movie_decode_state")));

void func_80016F5C(void);

void uploadTimImageWithOffset(u32 *tim, s16 *position);
void uploadTimArchive(TimArchive *archive, s16 *position);
void offsetTmdObjectTextures(u16 index, TmdObject *objects, ModelTextureOffset *offset);
void uploadImage64x256(u32 *pixels, s16 x, s16 y);
void resetPaletteRecords(void);
void releaseSavedImageEntry(u16 index);
u16 saveVram16Pixels(s32 x, s32 y);
void readVram16Pixels(s32 x, s32 y, u32 *pixels);
void setPaletteRecordColor(u16 index, u8 r, u8 g, u8 b);
void startPaletteCycle(u16 direction, u16 index, u8 first, u8 last, u8 interval);
s32 startPaletteTransition(s16 x, s16 y, u32 *pixels, u16 duration);
void requestSavedImageRestore(u16 index);
void initializePaletteTransition(void);
void updatePaletteTransition(void);
void startCurrentPlayerAnimation(u8 animation, u8 mode);
s32 updateCurrentPlayerAnimation(void);
void applyStereoSoundOption(void);
void initializeSoundSystem(s32 hot);
void loadSoundBank(s16 bankId, u8 *header, u8 *body);
void openMusicSequence(s16 bankId, u32 *sequence);
void closeMusicSequence(void);
void fadeOutMusicSequence(s16 step);
void stopMusicSequence(void);
void pauseMusicSequence(void);
void resumeMusicSequence(void);
void updateMusicSequenceFade(void);
void startSoundSystem(void);
void endSoundSystem(void);
s32 playSoundEffect(u8 bank, u16 programAndTone, s16 volume);
s32 getNearestCameraManhattanDistance(s32 x, s32 y, s32 z);
s32 attenuateSoundVolume(s32 distance, s16 volume);
s32 playPositionalSoundEffect(s32 x, s32 y, s32 z, u8 bank, u16 programAndTone, s16 volume);
s32 playOwnedSoundEffect(u8 owner, u8 bank, u16 note, s16 volume);
void stopMatchingSoundVoice(u8 owner, u8 bank, u16 note);
void playPlayerPositionalSoundEffect(u8 playerIndex, u8 bank, u16 programAndTone, s16 volume);
void initializeMemoryCardSaveHeader(void);
void captureMemoryCardSaveIcon(void);
void prepareMemoryCardSave(u8 slot, void *source, u32 size);
void selectMemoryCardLoadFile(u8 slot, void *destination);
s16 validateAndCopyMemoryCardSave(void);
void readMemoryCardDirectory(void);
s32 memoryCardFileExists(void);
s32 tryAcceptMemoryCard(void);
