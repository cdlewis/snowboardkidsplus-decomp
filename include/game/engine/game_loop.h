#pragma once
#include "game/graphics/coordinate.h"
#include "game/graphics/graphics.h"

void initializeGameSystems(void);
void runGameLoop(void);
extern MATRIX *D_8013AF9C;
extern u8 *D_8013AFA0, *D_8013AFA4;
extern s32 D_8013AF84;
extern GsOT D_800EC040[2], gPlayer1ModelOrderingTables[2];
extern GsOT D_800EC0E0[2], gPlayer2ModelOrderingTables[2], D_800EC1B0[2], D_800EC1D8[2];
extern MATRIX *D_800EC170[16];
extern u32 D_800EC200[8192], gPlayerOrderingTableTags[8192], gPlayer2Buffer1OrderingTableTags[4096];
extern u8 D_80100200[2][0x10000];

extern u16 gGameStateFlags;
extern u32 *D_8013AF70;
extern u32 *D_8013AF74;
