#pragma once

#include "common.h"

typedef struct PlayerResourceTextureSelections {
    u8 entries[2][15];
    u8 padding[2];
} PlayerResourceTextureSelections;

extern PlayerResourceTextureSelections gPlayerResourceTextureSelections;
extern u8 gDefaultPlayerResourceTextureSelections[15];

extern u8 gPlayerModelArchiveIndices[16];
extern s16 gPlayerResourceTexturePositions[4][2];
