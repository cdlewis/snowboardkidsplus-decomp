#pragma once

#include "decomp/types.h"

typedef struct SelectedRecord {
    u16 vertexIndices[3];
    s16 unk6;
    s16 unk8;
    s16 unkA;
} SelectedRecord;

extern SelectedRecord *D_8013B1F0 __attribute__((section(".data")));
extern s16 D_8013B1FE __attribute__((section(".data")));

