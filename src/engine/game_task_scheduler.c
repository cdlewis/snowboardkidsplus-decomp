#include "common.h"
#include "game/game_task_scheduler.h"

/* Shared with the JP scheduler and runGameLoop. Keep its initialized
 * storage at the original data address.
 */
s16 gFrameCounter = 0;
/* Unreferenced alignment padding, also present after the JP frame counter. */
static u16 sFrameCounterPadding = 0;
