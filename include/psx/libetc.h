#pragma once

#include "decomp/types.h"

typedef void (*DmaCallback)(void);
DmaCallback DMACallback(s32 channel, DmaCallback callback);

s32 ResetCallback(void);
s32 StopCallback(void);
s32 RestartCallback(void);
DmaCallback InterruptCallback(s32 channel, DmaCallback callback);
DmaCallback VSyncCallback(DmaCallback callback);
DmaCallback VSyncCallbacks(s32 channel, DmaCallback callback);
s32 CheckCallback(void);
s32 GetIntrMask(void);
s32 SetIntrMask(s32 mask);

s32 SetVideoMode(s32 mode);
s32 GetVideoMode(void);
