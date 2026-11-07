#pragma once

#include "game/random.h"
#include "game/graphics.h"
#include "game/task.h"
#include "psx/libgs.h"

typedef struct PlayerTimerState {
    GsCOORDINATE2 coordinate;
    u8 unk50[0x10];
    s16 unk60;
    u8 unk62[6];
} PlayerTimerState;

typedef struct RacePlayerVector {
    s16 x, y, z, pad;
} RacePlayerVector;

/* Partial fields, with the 0x9DC-byte stride used by the player loops. */
struct RacePlayer {
    GameTask *task;
    u16 playerIndex;
    u8 unk006[2];
    u8 unk008;
    u8 unk009;
    u8 unk00A[3];
    u8 unk00D;
    u8 unk00E[2];
    s32 unk010;
    u8 unk014;
    u8 unk015[7];
    s32 unk01C;
    s32 unk020;
    s8 unk024;
    u8 unk025[0xB];
    s32 x;
    s32 y;
    s32 z;
    u8 unk03C[0x14];
    s32 unk050;
    s32 unk054;
    s32 unk058;
    u8 unk05C[4];
    s16 unk060;
    s16 unk062;
    s16 unk064;
    u8 unk066[0xE];
    s32 unk074;
    s32 unk078;
    u8 unk07C[8];
    s16 unk084, unk086, unk088;
    u8 unk08A[2];
    s32 unk08C;
    s32 unk090;
    s16 unk094;
    s16 unk096;
    u16 unk098, unk09A, unk09C, unk09E;
    u16 unk0A0, unk0A2, unk0A4, unk0A6, unk0A8, unk0AA;
    s32 unk0AC;
    s32 unk0B0;
    u16 unk0B4;
    s16 unk0B6;
    s16 unk0B8;
    u8 unk0BA[2];
    s16 unk0BC;
    u16 unk0BE;
    u8 unk0C0[2];
    u16 unk0C2;
    u8 unk0C4[4];
    s16 unk0C8;
    s16 unk0CA[6];
    s16 unk0D6[6];
    u8 unk0E2[6];
    u16 unk0E8;
    u8 unk0EA[0xA];
    u16 unk0F4;
    s16 unk0F6;
    s16 unk0F8;
    u8 unk0FA[2];
    s16 unk0FC;
    u8 unk0FE;
    u8 unk0FF;
    u8 unk100[2];
    s16 unk102;
    union {
        s16 half[2];
        s32 word;
    } unk104;
    union {
        s16 half[2];
        s32 word;
    } unk108;
    u8 unk10C[4];
    u16 unk110;
    u8 unk112[4];
    s16 unk116;
    s16 unk118;
    s16 unk11A;
    u8 unk11C[6];
    s16 unk122;
    s16 unk124;
    u16 unk126;
    u32 unk128;
    u16 unk12C;
    u8 unk12E[2];
    s16 unk130;
    u8 unk132[2];
    s16 unk134;
    u8 unk136[2];
    u16 unk138;
    u8 unk13A[2];
    s16 unk13C;
    u8 unk13E[14];
    u8 unk14C[14];
    u8 unk15A[0x26];
    MATRIX effectMatrix180;
    u8 unk1A0[0x41C];
    RacePlayerVector unk5BC[14];
    RacePlayerVector unk62C[14];
    u8 unk69C[0x248];
    s32 position8E4[3];
    u8 unk8F0[0x2C];
    u16 *animationStream;
    u16 *animationFrame;
    u16 animationFrameCount;
    u16 animationTicks;
    u8 modelPartCount;
    u8 animationIndex;
    u8 animationFlags;
    u8 animationStep;
    u8 unk92C[2];
    s16 unk92E;
    u8 unk930[4];
    s8 unk934;
    s8 unk935;
    u8 unk936[0xD];
    u8 unk943;
    u8 randomIndex;
    u8 unk945;
    u8 unk946;
    u8 unk947[5];
    s32 unk94C;
    u8 unk950[4];
    u8 unk954;
    u8 unk955[5];
    s16 unk95A;
    s16 unk95C;
    s16 unk95E;
    s16 unk960;
    s16 unk962;
    u8 unk964;
    s8 unk965;
    u8 unk966[5];
    u8 soundVoice;
    s16 unk96C;
    u16 unk96E;
    u16 unk970;
    u8 unk972[2];
    PlayerTimerState timer;
};

/* Per-character animation indices; each row has the original 40-byte stride. */
extern u8 D_800EB09C[10][40];

extern RacePlayer D_80120E90[];
/* Shared result state; task code accesses the first word. */
extern s32 D_80130BF0[];
extern s8 D_8013AFD2 __attribute__((section(".data")));
/* GP-relative alias of D_8013AFD2. */
extern s8 activeRacePlayerCountGp;
extern s16 D_8013B0E4;
extern u16 D_8013B0E6;
extern s32 D_8013B0E8;
/* Two declarations preserve the original GP-relative and absolute accesses
 * to this shared pointer. The absolute alias is resolved by the symbol map. */
extern RacePlayer *D_8013B0D8;
extern RacePlayer *currentPlayerAbsolute
    __attribute__((section(".data")));
#ifdef RACE_PLAYER_ABSOLUTE_ADDRESSING
#define D_8013B0D8 currentPlayerAbsolute
#endif


void updateCurrentPlayerTaskCountdown(void);
s16 isFirstPlayerUsingCallback26D10(void);
void clearCurrentPlayerSequenceState(void);
void updateCurrentPlayerFromTaskPhase(void);

void loadCurrentPlayerModelPose(const s16 *values);
void setCurrentPlayerFinalPartTranslations(const s16 *values);
void copyPlayerPaletteRowWithTransition(s16 playerIndex, s32 unused, s32 paletteRow, u16 mode);
void copyPlayerPaletteRow(s16 playerIndex);
void clearCurrentPlayerTaskState(void);
void startPlayerTimerTask(RacePlayer *player);
void initPlayerTimerTask(void);
void initializePlayerParticleHistoryTask(void);
void fadeOutPlayerTimerTask(void);
void stopPlayerSound(u16 playerIndex);
s32 requestCurrentPlayerAction6(void);
s32 requestCurrentPlayerAction6NarrowAngle(void);
s32 requestCurrentPlayerAction7(void);
s32 requestCurrentPlayerAction17(void);
s16 countCurrentPlayerInputGroups(void);

void copySelectedRecordToCurrentPlayer(void);
s16 smoothAndScaleCurrentPlayerInput(s32 magnitude, s16 input);
void clampCurrentPlayerHorizontalVector(VECTOR *vector);

s32 checkCurrentPlayerFlags43040OrAngleRange(void);
s32 checkCurrentPlayerFlags345001(void);
s16 checkCurrentPlayerSurfaceType(void);

s32 checkCurrentPlayerFlags275000OrField134(void);

void addPlayerCappedCounters(u16 playerIndex, s32 amount);

void resetCurrentPlayerForCallback255C4(void);
void resetCurrentPlayerForCallback240D0(void);
void resetCurrentPlayerForCallback26DAC(void);
void resetCurrentPlayerForCallback205B8(void);

void playCurrentPlayerSoundEffect(s16 index);
void func_8002E5A0(u16 index, s16 value);

void startCurrentPlayerAnimation(u8 animation, u8 mode);

void func_8001E244(u32 *model);
void initializeCurrentPlayerModelAndHeight(void);
void initCurrentPlayerModelParts(void);
s32 callCurrentPlayerStateCallbackIfDifferent(u16 state);

void initializeActiveRacePlayers(void);

void assignTwoPlayerResultSlots(s16 result);


void updateActivePlayerRegionCollisions(void);

/* Advance a directional input sequence and return its completed entry result. */
u16 advanceCurrentPlayerInputSequence(u8 sequence[][2], s16 slot);
void createPlayerPositionTrailTask(s32 playerIndex);
void createPlayerRingEffectTask(s32 playerIndex);

s32 func_8004A764(s16 index, s32 x, s32 z);

void updatePlayerRenderSnapshots(void);

void assignCurrentPlayerFinishSlot(s32 taskValue);
void updateRacePlayerFinishSlotsTask(void);

struct Position3D;
u16 addPlayerFlagsIfWithinRadius(struct Position3D *position, s32 radius, s16 flags, u16 playerIndex);

void addPlayerFlagsWithinRadius(const Position3D *position, s32 radius, u16 flags);

u16 isPositionWithinPlayerCollisionRange(const Position3D *position, s32 radius, s32 height, s16 playerIndex);
