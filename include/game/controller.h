#pragma once

#include "decomp/types.h"

extern u16 D_8013AEE8[];

extern u8 gControllerFlags[4] __attribute__((section(".data")));

extern s16 D_8013AF08[2] __attribute__((section(".data.controller_values")));

void setControllerFlag5(u16 controller);
void setControllerFlag0(u16 controller);
s32 isControllerFlag5Clear(u16 controller);

/* Accept standard digital and analog controller input packets. */
s16 isControllerInputSupported(u16 controller);
