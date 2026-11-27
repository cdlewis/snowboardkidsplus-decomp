#pragma once

#include "decomp/types.h"

#define PADL1     (1 << 2)
#define PADL2     (1 << 0)
#define PADR1     (1 << 3)
#define PADR2     (1 << 1)
#define PADLup    (1 << 12)
#define PADLdown  (1 << 14)
#define PADLleft  (1 << 15)
#define PADLright (1 << 13)

/* Psy-Q LIBETC.H: word-indexed access to fixed scratchpad hardware RAM. */
#define getScratchAddr(offset) ((u32 *)(0x1F800000 + (offset) * 4))

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

s32 VSync(s32 mode);
