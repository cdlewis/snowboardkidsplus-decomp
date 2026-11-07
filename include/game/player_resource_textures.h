#pragma once

#include "common.h"

typedef struct PlayerResourceTextureSelections {
    u8 entries[2][15];
    u8 padding[2];
} PlayerResourceTextureSelections;

extern PlayerResourceTextureSelections gPlayerResourceTextureSelections;
extern u8 D_80123650[15];
