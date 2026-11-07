#pragma once

#include "decomp/types.h"

/* Centiseconds are stored with eight fractional bits. */
typedef struct RaceTime {
    s8 minutes;
    s8 seconds;
    s16 centiseconds;
} RaceTime;

extern RaceTime D_8013B154;
extern RaceTime D_8013B184;
extern RaceTime D_8013B190[];
extern RaceTime D_8013B198[];
extern RaceTime D_80124248[];
extern RaceTime gRaceElapsedTime;
extern RaceTime D_8013B188;
extern RaceTime D_80124050[];
extern RaceTime D_80124220[];

s16 compareRaceTimes(const RaceTime *a, const RaceTime *b);
void advanceRaceElapsedTime(void);
s32 calculateRaceTimeDifference(const RaceTime *a, const RaceTime *b, RaceTime *difference);

typedef char RaceTimeSizeCheck[(sizeof(RaceTime) == 0x4) ? 1 : -1];
