#pragma once

#include "common.h"

extern u8 gSaveRecordSavedOrLoaded;

void initializeSaveRecordTask(void);
void clearHalfwordPairTable(void);
void initializeMemoryCardSaveMenuTask(void);
void updateMemoryCardSaveMenuTask(void);
