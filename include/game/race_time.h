#pragma once

#include "decomp/types.h"

/* Centiseconds are stored with eight fractional bits. */
typedef struct RaceTime {
    s8 minutes;
    s8 seconds;
    s16 centiseconds;
} RaceTime;

extern RaceTime gRaceElapsedTime;
extern RaceTime D_8013B188;
extern RaceTime D_80124050[];

s16 compareRaceTimes(const RaceTime *a, const RaceTime *b);
void advanceRaceElapsedTime(void);
