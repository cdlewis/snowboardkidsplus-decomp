#pragma once

#include "game/random.h"
#include "game/task.h"

typedef struct RacePlayerVector {
    s16 x, y, z, pad;
} RacePlayerVector;

/* Partial fields, with the 0x9DC-byte stride used by the player loops. */
struct RacePlayer {
    GameTask *task;
    u8 unk004[0xBE];
    u16 unk0C2;
    u8 unk0C4[6];
    s16 unk0CA[6];
    s16 unk0D6[6];
    u8 unk0E2[0x34];
    s16 unk116;
    s16 unk118;
    s16 unk11A;
    u8 unk11C[0xC];
    u32 unk128;
    u16 unk12C;
    u8 unk12E[6];
    s16 unk134;
    u8 unk136[0x4F6];
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
void clearCurrentPlayerTaskState(void);

void copySelectedRecordToCurrentPlayer(void);

s32 checkCurrentPlayerFlags345001(void);

s32 checkCurrentPlayerFlags275000OrField134(void);
