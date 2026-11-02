#pragma once

#include "decomp/types.h"

typedef struct {
    u32 length;
    u32 *org;
    u32 offset, point;
    u32 *tag;
} GsOT;
