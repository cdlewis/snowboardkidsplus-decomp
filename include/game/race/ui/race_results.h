#pragma once

#include "decomp/types.h"

extern u8 gAttackTimeExpired __attribute__((section(".data")));
extern u8 gPlayerAttackTimeExpired[];

void initializeResultDisplayTask(void);
void initializeTaskFade(void);
void initializeTaskMenuFade(void);
void initializeTaskMenuFadeIn(void);
void initializeRaceResultsTask(void);
void initializeStatsMenuTask(void);
void initializeRaceTimeComparisonPanel(void);
