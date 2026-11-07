#include "game/io/controller.h"

/* Separate storage preserves absolute indexed accesses and small-data direct accesses. */
u16 D_8013AF0C[2] __attribute__((section(".sdata.controller_previous_buttons"))) = {0};
