#pragma once

#include "game/random.h"
#include "game/task.h"

typedef struct RacePlayerVector {
    s16 x, y, z, pad;
} RacePlayerVector;

/* Partial fields, with the 0x9DC-byte stride used by the player loops. */
struct RacePlayer {
    GameTask *task;
    u8 unk004[0xC6];
    s16 unk0CA[6];
    s16 unk0D6[6];
    u8 unk0E2[0x54A];
    RacePlayerVector unk62C[14];
    u8 unk69C[0x28C];
    u8 modelPartCount;
    u8 unk929[0x1B];
    u8 randomIndex;
    u8 unk945[0x97];
};

extern RacePlayer D_80120E90[];
/* Model code addresses this shared pointer absolutely; other player code
 * uses the small-data declaration. Both refer to the same storage. */
#ifdef RACE_PLAYER_ABSOLUTE_ADDRESSING
extern RacePlayer *D_8013B0D8 __attribute__((section(".data")));
#else
extern RacePlayer *D_8013B0D8;
#endif

void func_80026D10(void);
s16 isFirstPlayerUsingCallback26D10(void);
void clearCurrentPlayerSequenceState(void);

void setCurrentPlayerFinalPartTranslations(const s16 *values);
