#pragma once

#include "game/random.h"
#include "game/task.h"

typedef struct RacePlayerVector {
    s16 x, y, z, pad;
} RacePlayerVector;

/* Partial fields, with the 0x9DC-byte stride used by the player loops. */
struct RacePlayer {
    GameTask *task;
    u8 unk004[5];
    u8 unk009;
    u8 unk00A[3];
    u8 unk00D;
    u8 unk00E[0xE];
    s32 unk01C;
    s32 unk020;
    s8 unk024;
    u8 unk025[0x3D];
    s16 unk062;
    u8 unk064[0x14];
    s32 unk078;
    u8 unk07C[0x10];
    s32 unk08C;
    s32 unk090;
    s16 unk094;
    s16 unk096;
    u8 unk098[0x20];
    s16 unk0B8;
    u8 unk0BA[2];
    s16 unk0BC;
    u16 unk0BE;
    u8 unk0C0[2];
    u16 unk0C2;
    u8 unk0C4[6];
    s16 unk0CA[6];
    s16 unk0D6[6];
    u8 unk0E2[0x2E];
    u16 unk110;
    u8 unk112[4];
    s16 unk116;
    s16 unk118;
    s16 unk11A;
    u8 unk11C[0xA];
    u16 unk126;
    u32 unk128;
    u16 unk12C;
    u8 unk12E[6];
    s16 unk134;
    u8 unk136[2];
    u16 unk138;
    u8 unk13A[0x4F2];
    RacePlayerVector unk62C[14];
    u8 unk69C[0x28C];
    u8 modelPartCount;
    u8 unk929[0x1B];
    u8 randomIndex;
    u8 unk945[7];
    s32 unk94C;
    u8 unk950[4];
    u8 unk954;
    u8 unk955[0x10];
    s8 unk965;
    u8 unk966[0x76];
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
void copyPlayerPaletteRow(s16 playerIndex);
void clearCurrentPlayerTaskState(void);

void copySelectedRecordToCurrentPlayer(void);

s32 checkCurrentPlayerFlags43040OrAngleRange(void);
s32 checkCurrentPlayerFlags345001(void);

s32 checkCurrentPlayerFlags275000OrField134(void);

void addPlayerCappedCounters(u16 playerIndex, s32 amount);

void resetCurrentPlayerForCallback255C4(void);
void resetCurrentPlayerForCallback240D0(void);
void resetCurrentPlayerForCallback26DAC(void);
void resetCurrentPlayerForCallback205B8(void);

void func_8002E44C(s16 index);
void func_8002E5A0(u16 index, s16 value);

void func_80016E80(u8 index, u8 flags);
