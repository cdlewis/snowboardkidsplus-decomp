#pragma once

#include "psx/libgpu.h"

/* Two six-byte player records shared by the selection menus. */
typedef struct MenuSelectionState {
    s16 selection;
    s16 previousSelection;
    s16 unk04;
} MenuSelectionState;

extern MenuSelectionState D_801300AC[2];

void createPlayerGroupedSelectionTask(s32 player, s32 index);
extern RECT D_801300B8[4];

void initializeCountedSelectionBoundsTask(void);
void initializeCountedDisplayBoundsTask(void);

void initializeCountedGroupedSelectionBoundsTask(void);

void initializeCountedSplitSelectionBoundsTask(void);
void initializeCountedFourBoundsTask(void);

void updateSplitSelectionBoundsTransition(void);

void initializeCountedPlayerGroupedSelectionBoundsTask(void);
void drawCurrentTaskBoardStyleAndLevel(void);

/* Four-byte records indexed by the parent task's data1A. */
typedef struct TaskPanelSelectionValue {
    s16 value;
    s16 unk02;
} TaskPanelSelectionValue;

extern TaskPanelSelectionValue D_8013B508[];
void drawCurrentTaskSelectionPanel(void);
