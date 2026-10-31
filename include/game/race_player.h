#ifndef GAME_RACE_PLAYER_H
#define GAME_RACE_PLAYER_H

#include "game/random.h"

/* Partial PSX layout. The N64 RacePlayer layout cannot be used here. */
struct RacePlayer {
    u8 unk000[0x944];
    u8 randomIndex;
};

#endif
