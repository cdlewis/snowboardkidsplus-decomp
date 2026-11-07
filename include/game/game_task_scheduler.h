#pragma once

#include "decomp/types.h"

/* The scheduler owns the shared 12-bit frame counter. Its RNG consumers
 * use absolute addressing even though the scheduler itself accesses it via $gp.
 */
#ifdef FRAME_COUNTER_GP_ADDRESSING
extern s16 gFrameCounter;
#else
extern s16 gFrameCounter __attribute__((section(".data")));
#endif
