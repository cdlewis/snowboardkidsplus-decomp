#ifndef GAME_GAME_TASK_SCHEDULER_H
#define GAME_GAME_TASK_SCHEDULER_H

#include "decomp/types.h"

/* The scheduler owns the shared 12-bit frame counter. Its RNG consumers
 * use absolute addressing even though the scheduler itself accesses it via $gp.
 */
extern s16 gFrameCounter __attribute__((section(".data")));

#endif
