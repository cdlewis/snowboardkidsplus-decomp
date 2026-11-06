#pragma once

#include "decomp/types.h"

/* Shared CD state remains in the original data owner. */
extern s16 D_8013AE78;
extern s16 D_8013AE4C;
extern u16 gCdRequestPhase;
extern u16 gCdRequestReadIndex;
extern u16 gCdRequestWriteIndex;
extern u16 D_8013AE4E;
extern u16 D_8013AE50;

s32 isCdCommandQueueIdle(void);
s32 queueCdSectorCommand(s16 command, s16 sector, s16 count, void *destination, s32 value);
void queueStartupSectorReads(void);
void queueCdCommand(s16 command, s16 value);
void queueBackgroundTextureSectorCommands(void);
void func_80013DD0(s16 command, s16 value);
void resetPendingCdRequests(void);
void resetCdCommandState(void);

extern u8 gCdVolumeFadeActive;
extern u8 gCdVolumeFadeInterval;
extern s16 gCdVolumeFadeCurrent;
extern s16 gCdVolumeFadeTarget;
extern s16 gCdVolumeFadeStep;
extern s16 gCdVolumeFadeTimer;

void resetCdVolumeFade(void);
void startCdVolumeFade(s16 startVolume, s16 step, s16 targetVolume, u8 interval);
