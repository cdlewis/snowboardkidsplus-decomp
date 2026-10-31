#ifndef PSX_LIBETC_H
#define PSX_LIBETC_H

#include "decomp/types.h"

typedef void (*DmaCallback)(void);
DmaCallback DMACallback(s32 channel, DmaCallback callback);

s32 SetVideoMode(s32 mode);
s32 GetVideoMode(void);

#endif
