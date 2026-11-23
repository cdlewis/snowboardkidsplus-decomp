#pragma once

#include "psx/libgpu.h"

/* Two six-byte player records shared by the selection menus. */
typedef struct MenuSelectionState {
    s16 selection;
    s16 previousSelection;
    s16 unk04;
} MenuSelectionState;

extern MenuSelectionState D_801300AC[2];

s32 getCourseSelectionGroupIndex(void);
s16 mapCourseMenuByteIndex(const u8 *mapping, s16 index);
void createPlayerGroupedSelectionTask(s32 player, s32 index);
extern RECT D_801300B8[4];

void initializeCountedSelectionBoundsTask(void);
void initializeCountedTaskResourcePair(void);
void initializeAlternateCountedTaskResourcePair(void);
void initializeCountedTaskResourcePairEC50(void);
void initializeNumberAdjustmentTask(void);
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

typedef struct ByteMappingDescriptor {
    const u8 *mapping;
    u8 spacingX;
    u8 spacingY;
    u8 count;
    u8 unk07;
    const u8 *textMapping;
    const u8 *textureMapping;
    u8 unk10[4];
} ByteMappingDescriptor;

typedef struct SelectionItemStats {
    u8 rating;
    u16 price;
} SelectionItemStats;

extern SelectionItemStats D_8012FCE4[];
extern ByteMappingDescriptor D_8012FD10[];
extern const u16 *D_8012FFD4[];
void drawRegisteredSelectionItemStatus(void);
void drawSelectionItemDetails(void);
void drawSelectionPrizeAndPlayerCounters(void);
