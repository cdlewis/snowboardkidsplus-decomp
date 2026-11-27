#pragma once

#include "common.h"

extern u8 gPlayerModelPartParents[16] __attribute__((section(".data.player_model_parts")));
extern u8 gPlayerModelPartMirrorIndices[16] __attribute__((section(".data.player_model_parts")));
extern s16 gPlayerModelPartOffsets0[42] __attribute__((section(".data.player_model_parts")));
extern s16 gPlayerModelPartOffsets1[42] __attribute__((section(".data.player_model_parts")));
extern s16 gPlayerModelPartOffsets2[42] __attribute__((section(".data.player_model_parts")));
extern s16 gPlayerModelPartOffsets3[42] __attribute__((section(".data.player_model_parts")));
extern s16 gPlayerModelPartOffsets4[42] __attribute__((section(".data.player_model_parts")));
extern s16 *gPlayerCharacterModelPartOffsets[10] __attribute__((section(".data.player_model_parts")));
