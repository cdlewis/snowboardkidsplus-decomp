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
void func_80013DD0(s16 command, s16 value);
void resetPendingCdRequests(void);
