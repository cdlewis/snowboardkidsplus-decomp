#pragma once

#include "decomp/types.h"
#include "game/engine/game_task_scheduler.h"

typedef struct RacePlayer RacePlayer;

extern u8 gRandomTable[256];
extern u16 gMainRngIndex;
extern u16 gSecondaryRngIndex;

s32 randomNextMain(void);
s32 randomNextSecondary(void);
void resetSecondaryRng(void);
u8 randomNextObject(RacePlayer *player);
void resetGameplayRng(void);
