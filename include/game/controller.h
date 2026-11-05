#pragma once

#include "decomp/types.h"

extern u8 gControllerFlags[4] __attribute__((section(".data")));

void setControllerFlag0(u16 controller);
s32 isControllerFlag5Clear(u16 controller);
