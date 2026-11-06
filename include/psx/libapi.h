#pragma once

#include "decomp/types.h"

s32 SetRCnt(s32 specification, s16 target, s32 flags);
s32 GetRCnt(s32 specification);
s32 StartRCnt(s32 specification);
s32 StopRCnt(s32 specification);
s32 ResetRCnt(s32 specification);

u32 SetSp(u32 stack);
