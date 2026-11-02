#pragma once

#include "decomp/types.h"

/* Independent of the table-based gameplay RNG in game/random.h. */
extern s32 gLinearRandomState;
s32 randomNextLinear(void);
s32 randomLinearFromValue(s32 value);
