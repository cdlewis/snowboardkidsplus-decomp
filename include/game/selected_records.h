#pragma once

#include "common.h"

extern s16 D_8013AFCE __attribute__((section(".data")));

void resetSelectedRecordStates(void);
void updatePlayerField94CFromSelectedTable(u16 tableIndex, s16 sampleIndex,
                                          u16 playerIndex);
