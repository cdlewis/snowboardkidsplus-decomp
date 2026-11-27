#pragma once

#include "decomp/types.h"

/* Centiseconds are stored with eight fractional bits. */
typedef struct RaceTime {
    s8 minutes;
    s8 seconds;
    s16 centiseconds;
} RaceTime;

extern RaceTime gTrickAttackTimeLimit;
extern RaceTime gAttackTimeRemaining;
extern RaceTime gPlayerRaceTimes[];
extern RaceTime gPlayerAttackTimesRemaining[];
extern RaceTime gScoreAttackTimeLimits[];
extern RaceTime gRaceElapsedTime;
extern RaceTime D_8013B188;
extern RaceTime D_80124050[];
extern RaceTime gCourseCoinTargetTimes[];

struct GsOT;
void drawRaceTime(const RaceTime *value, s32 x, s32 y, s16 mode, struct GsOT *ot, u16 depth);

s16 compareRaceTimes(const RaceTime *a, const RaceTime *b);
void advanceRaceElapsedTime(void);
s32 calculateRaceTimeDifference(const RaceTime *a, const RaceTime *b, RaceTime *difference);

typedef char RaceTimeSizeCheck[(sizeof(RaceTime) == 0x4) ? 1 : -1];
