#pragma once

#include "psx/libgpu.h"

/* Shared by the selection menus; the last four halfwords remain unknown. */
typedef struct MenuSelectionState {
    s16 selection;
    s16 previousSelection;
    s16 unk04[4];
} MenuSelectionState;

extern MenuSelectionState D_801300AC;
extern RECT D_801300B8[4];

void initializeCountedSelectionBoundsTask(void);
void initializeCountedDisplayBoundsTask(void);

void initializeCountedGroupedSelectionBoundsTask(void);

void initializeCountedSplitSelectionBoundsTask(void);
void initializeCountedFourBoundsTask(void);

void updateSplitSelectionBoundsTransition(void);
