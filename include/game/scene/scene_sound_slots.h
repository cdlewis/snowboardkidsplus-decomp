#pragma once

#include "common.h"

typedef struct SceneSoundSlot {
    u16 soundKey;
    u8 voice;
    u8 unk03;
} SceneSoundSlot;

extern SceneSoundSlot gSceneSoundSlots[8];
/* Storage remains in the original menu/selection_tasks data owner. */
extern u16 gSceneOneShotSoundKey __attribute__((section(".sdata.scene_sound_state")));
extern u8 gSceneOneShotVoice __attribute__((section(".sdata.scene_sound_state")));
