#include "common.h"
#include "game/game_task_scheduler.h"

/* Shared with the JP scheduler; the PSX update loop is still in assembly
 * (func_8001A540). Keep its initialized storage at the original data address.
 */
s16 gFrameCounter = 0;
/* Unreferenced alignment padding, also present after the JP frame counter. */
static u16 sFrameCounterPadding = 0;
