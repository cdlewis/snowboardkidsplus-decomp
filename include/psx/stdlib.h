#pragma once

#include "decomp/types.h"

int rand(void);

/* Call the SDK entry point without GCC expanding the abs builtin. */
s32 sdkAbs(s32 value) __asm__("abs");
