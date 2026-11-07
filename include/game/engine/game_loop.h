#pragma once
#include "psx/libgs.h"
#include "psx/libgpu.h"

void initializeGameSystems(void);
void runGameLoop(void);
extern VECTOR *D_8013AF8C;
extern SVECTOR *D_8013AF94;
extern MATRIX *D_8013AF9C;
extern u8 *D_8013AFA0, *D_8013AFA4;
extern s32 D_8013AF84;
extern GsOT D_800EC040[2], D_800EC068[2], D_800EC090[2], D_800EC0B8[2];
extern GsOT D_800EC0E0[2], D_800EC108[2], D_800EC130[2], D_800EC1B0[2], D_800EC1D8[2];
extern RECT D_800EC158[3];
extern MATRIX *D_800EC170[16];
extern u32 D_800EC200[8192], D_800F4200[8192], D_800FC200[4096];
extern u8 D_80100200[2][0x10000];

#ifdef GAME_LOOP_ABSOLUTE_ADDRESSING
extern u16 D_8013AFAA __attribute__((section(".data")));
#else
extern u16 D_8013AFAA;
#endif
extern u32 *D_8013AF70, *D_8013AF74;
extern s16 D_8013AF7E;
extern s16 D_8013AF80;
