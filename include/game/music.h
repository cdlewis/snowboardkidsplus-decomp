#pragma once

#include "decomp/types.h"

/* Shared with the sequence loading and playback routines still in assembly. */
/* Presentation tasks use absolute addressing; other callers use $gp. */
#ifdef MUSIC_STATE_ABSOLUTE_ADDRESSING
extern s16 D_8013B106 __attribute__((section(".data")));
#else
extern s16 D_8013B106;
#endif
#ifdef MUSIC_ABSOLUTE_ADDRESSING
#define MUSIC_DATA __attribute__((section(".data")))
#else
#define MUSIC_DATA
#endif
extern s16 gMusicSequenceId MUSIC_DATA;
extern u8 gMusicSequenceActive MUSIC_DATA;
/* Signed volume with eight fractional bits, shared with the fade updater. */
extern s16 D_8013AF30 MUSIC_DATA;
#undef MUSIC_DATA
extern s8 gSoundBankLoaded[12];
extern s8 gSoundSystemActive;
extern u8 gMusicFadeMode;
extern s16 gMusicFadeStep;

void playMusicSequence(s16 count, u8 left, u8 right);
void fadeOutMusicSequence(s16 step);
void updateMusicSequenceFade(void);
void func_80017E7C(s16 repeatCount);
void setMusicSequenceEnabled(s16 enabled);
void stopMusicSequence(void);
void loadSoundBank(s16 bankId, u8 *header, u8 *body);
void openMusicSequence(s16 bankId, u32 *sequence);
void closeMusicSequence(void);
void pauseMusicSequence(void);
void resumeMusicSequence(void);
void applyStereoSoundOption(void);
void startSoundSystem(void);
void endSoundSystem(void);

/* Apply distance attenuation to a sound volume. */
s32 attenuateSoundVolume(s32 distance, s16 volume);

/* Returns the allocated voice, or 0xFF when the bank is unavailable. */
s32 playSoundEffect(u8 bank, u16 programAndTone, s16 volume);

s32 func_800183FC(u16 playerIndex, u8 bank, u16 sound, s16 volume);

/* Positional counterpart of func_800181B0; returns the voice or 0xFF. */
/* Positional counterpart of playSoundEffect; returns the voice or 0xFF. */
s32 func_80018538(s32 x, s32 y, s32 z, u8 bank, u16 programAndTone, s16 volume);

/* Fade the serial audio volume from start toward end by step per update. */
void startCdVolumeFade(s16 start, s16 step, s16 end, u8 mode);
void func_800187B8(u8 voice);

void stopMatchingSoundVoice(u8 owner, u8 bank, u16 note);
