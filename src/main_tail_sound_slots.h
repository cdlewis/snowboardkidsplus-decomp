#pragma once

#include "common.h"

typedef struct TextSoundSlot {
    u16 soundKey;
    u8 voice;
    u8 unk03;
} TextSoundSlot;

extern TextSoundSlot gTextSoundSlots[8];
