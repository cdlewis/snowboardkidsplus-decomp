#pragma once

#include "decomp/types.h"

/* Shared with the sequence loading and playback routines still in assembly. */
extern s16 gMusicSequenceId;
/* Presentation tasks use absolute addressing; other callers use $gp. */
#ifdef MUSIC_STATE_ABSOLUTE_ADDRESSING
extern s16 D_8013B106 __attribute__((section(".data")));
#else
extern s16 D_8013B106;
#endif
extern u8 gMusicSequenceActive;
extern s8 gSoundSystemActive;
extern u8 gMusicFadeMode;
extern s16 gMusicFadeStep;

void fadeOutMusicSequence(s16 step);
void func_80017E7C(s16 repeatCount);
void setMusicSequenceEnabled(s16 enabled);
void stopMusicSequence(void);
void pauseMusicSequence(void);
void resumeMusicSequence(void);
void endSoundSystem(void);

/* Returns the allocated voice, or 0xFF when the bank is unavailable. */
s32 func_800181B0(u8 bank, u16 programAndTone, s16 volume);

/* Positional counterpart of func_800181B0; returns the voice or 0xFF. */
s32 func_80018538(s32 x, s32 y, s32 z, u8 bank, u16 programAndTone, s16 volume);

/* Fade the serial audio volume from start toward end by step per update. */
void func_80013294(s16 start, s16 step, s16 end, u8 mode);
void func_800187B8(u8 voice);
