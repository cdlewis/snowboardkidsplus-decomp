#pragma once
#include "game/graphics/coordinate.h"
#include "game/graphics/graphics.h"

void initializeGameSystems(void);
void runGameLoop(void);
extern u8 *D_8013AFA0, *D_8013AFA4;
extern s32 gProjectionDistance;
extern GsOT gPlayer1Layer7OrderingTables[2], gPlayer1ModelOrderingTables[2];
extern GsOT gPlayer2Layer7OrderingTables[2], gPlayer2ModelOrderingTables[2], gPlayer1DepthOffsetOrderingTables[2], gPlayer2DepthOffsetOrderingTables[2];
extern MATRIX *gScratchModelMatrices[16];
extern u32 gMainOrderingTableTags[8192], gPlayerOrderingTableTags[8192], gPlayer2Buffer1OrderingTableTags[4096];
extern PACKET gGraphicsPacketBuffers[2][0x10000];

extern u16 gGameStateFlags;
extern u32 *gCurrentPlayerDepthOffsetOrderingTable;
extern u32 *gCurrentPlayerAlternateLayerOrderingTable;
