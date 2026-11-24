#pragma once

#include "decomp/types.h"

extern u8 D_8013B18D __attribute__((section(".data")));
extern u8 D_8013B1A0[];

void initializeResultDisplayTask(void);
void initializeTaskFade(void);
void initializeTaskMenuFade(void);
void initializeTaskMenuFadeIn(void);
void initializeRaceResultsTask(void);
void initializeStatsMenuTask(void);
void initializeRaceTimeComparisonPanel(void);
