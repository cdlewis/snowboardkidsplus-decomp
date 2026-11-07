#pragma once

#include "common.h"

typedef struct UnlockMenuState {
    s16 selection, previousSelection;
    s32 unk04[7];
    s32 unk20[7];
    s32 unk3C[7];
    s32 unk58[7];
    s16 unk74[4];
    u8 unk7C[0x10];
    s16 unk8C;
    u8 unk8E[0x72];
} UnlockMenuState;

typedef char UnlockMenuStateSizeCheck[(sizeof(UnlockMenuState) == 0x100) ? 1 : -1];

void resetUnlockMenuOffsets(UnlockMenuState *state);
void resetUnlockMenuAnimation(UnlockMenuState *state);
