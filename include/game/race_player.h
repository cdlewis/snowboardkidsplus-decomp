#pragma once

#include "game/random.h"
#include "game/task.h"

/* Partial fields, with the 0x9DC-byte stride used by the player loops. */
struct RacePlayer {
    GameTask *task;
    u8 unk004[0x940];
    u8 randomIndex;
    u8 unk945[0x97];
};

extern RacePlayer D_80120E90[];

void func_80026D10(void);
s16 isFirstPlayerUsingCallback26D10(void);
