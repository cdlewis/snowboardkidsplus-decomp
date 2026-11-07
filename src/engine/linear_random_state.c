#include "game/engine/linear_random.h"

/* Kept separate so the update routine retains its original GP-relative access. */
s32 gLinearRandomState __attribute__((section(".data.linear_random_state"))) = 1;
