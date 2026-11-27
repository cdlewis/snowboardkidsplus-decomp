#pragma once

#include "common.h"

extern u8 gSaveRecordSavedOrLoaded;
extern const u16 gMemoryCardOperationFailedText[];

void initializeSaveRecordTask(void);
void clearHalfwordPairTable(void);
void initializeMemoryCardSaveMenuTask(void);
void updateMemoryCardSaveMenuTask(void);
