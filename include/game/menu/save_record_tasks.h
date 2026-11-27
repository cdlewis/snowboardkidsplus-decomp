#pragma once

#include "common.h"

extern u8 gSaveRecordSavedOrLoaded;
extern const u16 gMemoryCardOperationFailedText[];
extern const u16 gSaveRecordConfirmationSprites[];
extern const u16 gMemoryCardSlotChoiceSprites[];

void initializeSaveRecordTask(void);
void clearHalfwordPairTable(void);
void initializeMemoryCardSaveMenuTask(void);
void updateMemoryCardSaveMenuTask(void);
