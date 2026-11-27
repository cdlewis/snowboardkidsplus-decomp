#pragma once

#include "psx/libgpu.h"

/* Two six-byte player records shared by the selection menus. */
typedef struct MenuSelectionState {
    s16 selection;
    s16 previousSelection;
    s16 blinkTimer;
} MenuSelectionState;

typedef struct MenuModelSelectionPair {
    s16 selection;
    s16 previousSelection;
} MenuModelSelectionPair;

/* The shop task remembers a one-based choice. Zero selects its opening greeting. */
extern volatile u8 gBoardShopRememberedSelection;
extern volatile u8 boardShopRememberedSelectionAbsolute __attribute__((section(".data")));

/* Zero opens the greeting; other values remember a one-based menu choice. */
extern u8 gMoneyPanelRememberedSelection;
extern u8 boardMenuRememberedSelectionAbsolute __attribute__((section(".data")));

extern MenuModelSelectionPair gMenuPlayerTextureSelections[];
extern const u8 gMenuPlayerTextureIds[16];
extern MenuSelectionState gMenuSelectionStates[2];

s32 getCourseSelectionGroupIndex(void);
s16 mapCourseMenuByteIndex(const u8 *mapping, s16 index);
void createPlayerGroupedSelectionTask(s32 player, s32 index);
extern RECT gMenuModelDrawAreas[4];

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

extern TaskPanelSelectionValue gTaskPanelSelectionValues[];
void drawCurrentTaskSelectionPanel(void);

typedef struct ByteMappingDescriptor {
    const u8 *mapping;
    u8 spacingX;
    u8 spacingY;
    u8 count;
    u8 unk07;
    const u8 *textMapping;
    const u8 *textureMapping;
    u32 unk10;
} ByteMappingDescriptor;

typedef struct SelectionItemStats {
    u8 rating;
    u16 price;
} SelectionItemStats;

extern SelectionItemStats gCourseMenuItemStats[];
extern ByteMappingDescriptor gCourseMenuMappingGroups[4];
extern const u16 *gSelectionPanelTexts[10];
void drawRegisteredSelectionItemStatus(void);
void drawSelectionItemDetails(void);
void drawSelectionPrizeAndPlayerCounters(void);

extern const u8 gCourseMenuGroup0Mapping[8];
extern const u8 gCourseMenuGroup1Mapping[8];
extern const u8 gCourseMenuGroup0TextureMapping[8];
extern const u8 gCourseMenuGroup1TextureMapping[8];
extern const u16 *gSelectionPrizeDescriptionTexts[10];
