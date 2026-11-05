#pragma once

#include "decomp/types.h"

/* Pending effect IDs remain in the shared controller data object. */
#ifdef CONTROLLER_EFFECTS_ABSOLUTE_ADDRESSING
extern s16 D_8013AF08[2] __attribute__((section(".data")));
#else
extern s16 D_8013AF08[];
#endif

void resetControllerEffects(void);
