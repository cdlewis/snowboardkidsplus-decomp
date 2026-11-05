#pragma once

#include "decomp/types.h"

/* Shared with the sequence loading and playback routines still in assembly. */
extern s16 gMusicSequenceId;
extern u8 gMusicSequenceActive;
extern s8 gSoundSystemActive;
extern u8 gMusicFadeMode;
extern s16 gMusicFadeStep;

void fadeOutMusicSequence(s16 step);
void stopMusicSequence(void);
void pauseMusicSequence(void);
void resumeMusicSequence(void);
void endSoundSystem(void);
