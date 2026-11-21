#include "panel_tasks_internal.h"
#include "game/menu/task_widgets.h"
#include "game/menu/selected_sprite.h"
#include "game/engine/game_task_scheduler.h"
#include "grouped_selection_internal.h"
#include "game/audio/music.h"
#include "game/engine/task.h"
#include "game/graphics/coordinate.h"
#include "game/graphics/graphics.h"
#include "game/io/cd_queue.h"
#include "game/io/dat_entries.h"
#include "game/io/dat_resources.h"
#include "game/menu/menu_selection_state.h"
#include "game/menu/movement.h"
#include "game/race/player/race_player.h"
#include "game/save/save_data.h"
#include "psx/libgpu.h"
#include "psx/libgs.h"

#define GS_MODEL_OBJECT_HIDDEN 0x80000000u
#define TASK_POSITION(task) ((Movement2D *)&D_800D9504[(task)->resource14])

#define CHILDREN_READY(entries) (!*(s16 *)&(entries)[0].flags && !*(s16 *)&(entries)[1].flags && \
                                 !*(s16 *)&(entries)[2].flags && !*(s16 *)&(entries)[3].flags && \
                                 !*(s16 *)&(entries)[4].flags)
#define CHILD_MOVEMENT(entry) ((Movement2D *)&D_800D9504[(entry).task->resource14])
#define MODEL_PREVIEW_MOVEMENT(entry) ((Movement2D *)&D_800D9504[(entry).task->resource16])
#define BOARD_PURCHASE_CHILDREN_READY(entries) \
    ((s16)(entries)[0].flags == 0 && (s16)(entries)[1].flags == 0 && \
     (s16)(entries)[2].flags == 0 && (s16)(entries)[4].flags == 0)


typedef struct GroupedSelectionDescriptor {
    const u8 *mapping;
    u8 spacingX;
    u8 spacingY;
    u8 count;
    u8 unk07;
    const u8 *label;
    u32 unk0C;
    u8 *tripletIndices;
} GroupedSelectionDescriptor;

typedef struct GridModelPreviewState {
    s16 unk00;
    s16 rotationStep;
    s16 unk04;
    s16 unk06;
} GridModelPreviewState;

typedef struct MoneyMenuSelection {
    s16 mode;
    s16 selectedIndex;
    s16 previousIndex;
    s16 choiceResult;
} MoneyMenuSelection;

extern s16 gFrameCounter __attribute__((section(".data")));
extern u16 D_8013AF78 __attribute__((section(".data")));
extern GameTask *D_8013AE88 __attribute__((section(".data")));
extern GameTask *D_8013AE8C __attribute__((section(".data")));
extern GameTask *D_8013AE98 __attribute__((section(".data")));
extern u8 D_8013B3A4 __attribute__((section(".data")));
extern u8 D_8013B3A5 __attribute__((section(".data")));
extern u8 D_8013B3A6 __attribute__((section(".data")));

static const u32 sBoardSelectionDetailsJumpTablePadding
    __attribute__((section(".rodata.board_selection_details_padding"))) = 0;

extern const u8 D_80131868[];
extern GroupedSelectionDescriptor D_801318D4[];
extern u8 D_8013B4BC __attribute__((section(".data")));
extern volatile u8 shopMenuRememberedSelectionGp;
extern const u8 D_801317F0[];
extern const u8 D_8013B4A0[];
extern const u8 D_8013B4B4[];
extern u8 D_8013B4BD;
extern u8 boardMenuRememberedSelectionAbsolute __attribute__((section(".data")));

extern const u16 D_8013143C[], D_80131498[], D_801314FC[], D_80131548[], D_80131574[];
extern const u16 D_8013158C[], D_801315A4[], D_801315C0[], D_801315E0[], D_80131630[];
extern const u16 D_8013167C[], D_801316BC[], D_801316E8[], D_80131728[], D_80131760[];

extern const s16 D_80131A9C[], D_80131B08[], D_80131B74[];
extern const s16 D_80131BE0[], D_80131C4C[], D_80131CB8[];

static const GridSpriteEntry sShopMenuSprites[4]
    __attribute__((section(".data.shop_menu_sprites"), aligned(4))) = {
    {0xD6, 0, {0, 0, 0}},
    {0xD5, 0, {0, 0, 0}},
    {0xD7, 0, {0, 0, 0}},
    {0xC6, 0, {0, 0, 0}},
};

static const u16 *sShopMenuTexts[15] __attribute__((section(".data.shop_menu_texts"))) = {
    D_8013143C, D_80131498, D_801314FC, D_80131548, D_80131574,
    D_8013158C, D_801315A4, D_801315C0, D_801315E0, D_80131630,
    D_8013167C, D_801316BC, D_801316E8, D_80131728, D_80131760,
};

static const u8 sShopMenuSpriteMapping[4]
    __attribute__((section(".data.shop_menu_mappings"), aligned(4))) = {0, 1, 2, 3};
static const u8 sShopMenuTextMapping[4]
    __attribute__((section(".data.shop_menu_mappings"), aligned(4))) = {0, 1, 2, 3};
static const u8 sShopMenuCharacter8TextMapping[4]
    __attribute__((section(".data.shop_menu_mappings"), aligned(4))) = {13, 1, 2, 3};
static const u8 sShopMenuCharacter9TextMapping[4]
    __attribute__((section(".data.shop_menu_mappings"), aligned(4))) = {14, 1, 2, 3};

static const u8 sShopMenuParentStates[4]
    __attribute__((section(".data.shop_menu_destinations"), aligned(4))) = {12, 13, 14, 6};
static const u8 sShopMenuTextureMapping[4]
    __attribute__((section(".data.shop_menu_destinations"), aligned(4))) = {7, 7, 7, 6};

/* The remembered choice follows the adjacent three-entry menu's sprite mapping. */
static const u8 sMoneyPanelSelectionSpriteMapping[3]
    __attribute__((section(".data.shop_menu_selection"))) = {0, 1, 2};
volatile u8 gShopMenuRememberedSelection __attribute__((section(".data.shop_menu_selection"))) = 0;

static const u8 sBoardPurchaseSpriteMapping[4]
    __attribute__((section(".data.board_purchase_mappings"), aligned(4))) = {1, 2, 3, 5};
static const u8 sBoardPurchaseFullSpriteMapping[8]
    __attribute__((section(".data.board_purchase_mappings"), aligned(4))) = {1, 2, 3, 4, 5, 0, 0, 0};

const s16 *gGridModelPreviewPoses[10]
    __attribute__((section(".data.grid_model_preview_poses"))) = {
    D_80131A9C, D_80131B08, D_80131A9C, D_80131B08, D_80131B74,
    D_80131B08, D_80131B08, D_80131CB8, D_80131BE0, D_80131C4C
};

PanelModeLabel gRegisteredPanelMode0Label __attribute__((section(".data.registered_panel_mode_labels"))) = {2, 0};
PanelModeLabel gRegisteredPanelMode1Label __attribute__((section(".data.registered_panel_mode_labels"))) = {3, 0};

u8 gBoardSelectionLevels[12] __attribute__((section(".data.board_selection_levels"))) = {
    1, 1, 1, 2, 2, 2, 3, 3, 3, 1, 2, 3
};

s8 *gBoardStyleText[4] __attribute__((section(".data.board_style_text"))) = {
    D_80011DC8, D_80011DC8 + 0x10, D_80011DC8 + 0x20, D_80011DC8 + 0x2C
};

u8 gBoardSelectionStyles[12] __attribute__((section(".data.board_selection_styles"))) = {
    0, 1, 2, 0, 1, 2, 0, 1, 2, 3, 3, 3
};

const u8 gSelectedBoardLevelLabel[12] __attribute__((section(".rodata.selected_board_details"), aligned(4))) = {
    0x82, 0x6B, 0x82, 0x64, 0x82, 0x75, 0x82, 0x64, 0x82, 0x6B, 0, 0
};

const u8 gSelectedBoardPricePlaceholder[12] __attribute__((section(".rodata.selected_board_details"), aligned(4))) = {
    0x81, 0x7C, 0x81, 0x7C, 0x81, 0x7C, 0x81, 0x7C, 0x81, 0x7C, 0, 0
};

const u32 gGroupedSelectionBoundsTablePadding
    __attribute__((section(".rodata.grouped_selection_bounds_padding"))) = 0;

const u8 gBoardLevelLabel[16] __attribute__((section(".rodata.board_level_label"), aligned(4))) = {
    0x82, 0x6B, 0x82, 0x64, 0x82, 0x75, 0x82, 0x64,
    0x82, 0x6B, 0, 0, 0, 0, 0, 0
};

/* Read the encoded byte before converting it to a signed screen offset. */
static volatile u8 sBoardPriceVerticalOffsets[12]
    __attribute__((section(".data.board_price_vertical_offsets"), aligned(4))) = {
    0xF8, 0xF8, 0xF6, 0xF8, 0xE8, 0xF6, 0xF6, 0xEC, 0xFC, 0xF9, 0, 0
};

u16 gCurrentTaskPanelText[8] __attribute__((section(".data.current_task_panel_text"))) = {
    0xFA, 0x94, 0xB1, 0x9F, 0x98, 0x31, 0, 0
};

const u16 gTaskSelectionPanelLabel[6] __attribute__((section(".data.task_selection_panel_text"), aligned(4))) = {
    0x18, 0x157, 0x2D, 0x8020, 0xBF, 0
};

const u16 gTaskSelectionPanelText[8] __attribute__((section(".data.task_selection_panel_text"), aligned(4))) = {
    0xFA, 0x94, 0xB1, 0x9E, 0xA2, 0x33, 0, 0
};

ResourceSectorRange gGridResourceSecondSectorRanges[10][5] __attribute__((section(".data.grid_resource_sector_ranges"))) = {
    {
        {DAT_GRID_TEXTURE_0_0_SECTOR, DAT_GRID_TEXTURE_0_0_SECTOR_COUNT},
        {DAT_GRID_TEXTURE_0_1_SECTOR, DAT_GRID_TEXTURE_0_1_SECTOR_COUNT},
        {DAT_GRID_TEXTURE_0_2_SECTOR, DAT_GRID_TEXTURE_0_2_SECTOR_COUNT},
        {DAT_GRID_TEXTURE_0_3_SECTOR, DAT_GRID_TEXTURE_0_3_SECTOR_COUNT},
        {DAT_GRID_TEXTURE_0_4_SECTOR, DAT_GRID_TEXTURE_0_4_SECTOR_COUNT},
    },
    {
        {DAT_GRID_TEXTURE_1_0_SECTOR, DAT_GRID_TEXTURE_1_0_SECTOR_COUNT},
        {DAT_GRID_TEXTURE_1_1_SECTOR, DAT_GRID_TEXTURE_1_1_SECTOR_COUNT},
        {DAT_GRID_TEXTURE_1_2_SECTOR, DAT_GRID_TEXTURE_1_2_SECTOR_COUNT},
        {DAT_GRID_TEXTURE_1_3_SECTOR, DAT_GRID_TEXTURE_1_3_SECTOR_COUNT},
        {DAT_GRID_TEXTURE_1_4_SECTOR, DAT_GRID_TEXTURE_1_4_SECTOR_COUNT},
    },
    {
        {DAT_GRID_TEXTURE_2_0_SECTOR, DAT_GRID_TEXTURE_2_0_SECTOR_COUNT},
        {DAT_GRID_TEXTURE_2_1_SECTOR, DAT_GRID_TEXTURE_2_1_SECTOR_COUNT},
        {DAT_GRID_TEXTURE_2_2_SECTOR, DAT_GRID_TEXTURE_2_2_SECTOR_COUNT},
        {DAT_GRID_TEXTURE_2_3_SECTOR, DAT_GRID_TEXTURE_2_3_SECTOR_COUNT},
        {DAT_GRID_TEXTURE_2_4_SECTOR, DAT_GRID_TEXTURE_2_4_SECTOR_COUNT},
    },
    {
        {DAT_GRID_TEXTURE_3_0_SECTOR, DAT_GRID_TEXTURE_3_0_SECTOR_COUNT},
        {DAT_GRID_TEXTURE_3_1_SECTOR, DAT_GRID_TEXTURE_3_1_SECTOR_COUNT},
        {DAT_GRID_TEXTURE_3_2_SECTOR, DAT_GRID_TEXTURE_3_2_SECTOR_COUNT},
        {DAT_GRID_TEXTURE_3_3_SECTOR, DAT_GRID_TEXTURE_3_3_SECTOR_COUNT},
        {DAT_GRID_TEXTURE_3_4_SECTOR, DAT_GRID_TEXTURE_3_4_SECTOR_COUNT},
    },
    {
        {DAT_GRID_TEXTURE_4_0_SECTOR, DAT_GRID_TEXTURE_4_0_SECTOR_COUNT},
        {DAT_GRID_TEXTURE_4_1_SECTOR, DAT_GRID_TEXTURE_4_1_SECTOR_COUNT},
        {DAT_GRID_TEXTURE_4_2_SECTOR, DAT_GRID_TEXTURE_4_2_SECTOR_COUNT},
        {DAT_GRID_TEXTURE_4_3_SECTOR, DAT_GRID_TEXTURE_4_3_SECTOR_COUNT},
        {DAT_GRID_TEXTURE_4_4_SECTOR, DAT_GRID_TEXTURE_4_4_SECTOR_COUNT},
    },
    {
        {DAT_GRID_TEXTURE_5_0_SECTOR, DAT_GRID_TEXTURE_5_0_SECTOR_COUNT},
        {DAT_GRID_TEXTURE_5_1_SECTOR, DAT_GRID_TEXTURE_5_1_SECTOR_COUNT},
        {DAT_GRID_TEXTURE_5_2_SECTOR, DAT_GRID_TEXTURE_5_2_SECTOR_COUNT},
        {DAT_GRID_TEXTURE_5_3_SECTOR, DAT_GRID_TEXTURE_5_3_SECTOR_COUNT},
        {DAT_GRID_TEXTURE_5_4_SECTOR, DAT_GRID_TEXTURE_5_4_SECTOR_COUNT},
    },
    {
        {DAT_GRID_TEXTURE_6_0_SECTOR, DAT_GRID_TEXTURE_6_0_SECTOR_COUNT},
        {DAT_GRID_TEXTURE_6_1_SECTOR, DAT_GRID_TEXTURE_6_1_SECTOR_COUNT},
        {DAT_GRID_TEXTURE_6_2_SECTOR, DAT_GRID_TEXTURE_6_2_SECTOR_COUNT},
        {DAT_GRID_TEXTURE_6_3_SECTOR, DAT_GRID_TEXTURE_6_3_SECTOR_COUNT},
        {DAT_GRID_TEXTURE_6_4_SECTOR, DAT_GRID_TEXTURE_6_4_SECTOR_COUNT},
    },
    {
        {DAT_GRID_TEXTURE_7_0_SECTOR, DAT_GRID_TEXTURE_7_0_SECTOR_COUNT},
        {DAT_GRID_TEXTURE_7_1_SECTOR, DAT_GRID_TEXTURE_7_1_SECTOR_COUNT},
        {DAT_GRID_TEXTURE_7_2_SECTOR, DAT_GRID_TEXTURE_7_2_SECTOR_COUNT},
        {DAT_GRID_TEXTURE_7_3_SECTOR, DAT_GRID_TEXTURE_7_3_SECTOR_COUNT},
        {DAT_GRID_TEXTURE_7_4_SECTOR, DAT_GRID_TEXTURE_7_4_SECTOR_COUNT},
    },
    {
        {DAT_GRID_TEXTURE_8_0_SECTOR, DAT_GRID_TEXTURE_8_0_SECTOR_COUNT},
        {DAT_GRID_TEXTURE_8_1_SECTOR, DAT_GRID_TEXTURE_8_1_SECTOR_COUNT},
        {DAT_GRID_TEXTURE_8_2_SECTOR, DAT_GRID_TEXTURE_8_2_SECTOR_COUNT},
        {DAT_GRID_TEXTURE_8_3_SECTOR, DAT_GRID_TEXTURE_8_3_SECTOR_COUNT},
        {DAT_GRID_TEXTURE_8_4_SECTOR, DAT_GRID_TEXTURE_8_4_SECTOR_COUNT},
    },
    {
        {DAT_GRID_TEXTURE_9_0_SECTOR, DAT_GRID_TEXTURE_9_0_SECTOR_COUNT},
        {DAT_GRID_TEXTURE_9_1_SECTOR, DAT_GRID_TEXTURE_9_1_SECTOR_COUNT},
        {DAT_GRID_TEXTURE_9_2_SECTOR, DAT_GRID_TEXTURE_9_2_SECTOR_COUNT},
        {DAT_GRID_TEXTURE_9_3_SECTOR, DAT_GRID_TEXTURE_9_3_SECTOR_COUNT},
        {DAT_GRID_TEXTURE_9_4_SECTOR, DAT_GRID_TEXTURE_9_4_SECTOR_COUNT},
    }
};

ResourceSectorRange gGridResourceFirstSectorRanges[10][5] __attribute__((section(".data.grid_resource_sector_ranges"))) = {
    {
        {DAT_GRID_MODEL_0_0_SECTOR, DAT_GRID_MODEL_0_0_SECTOR_COUNT},
        {DAT_GRID_MODEL_0_1_SECTOR, DAT_GRID_MODEL_0_1_SECTOR_COUNT},
        {DAT_GRID_MODEL_0_2_SECTOR, DAT_GRID_MODEL_0_2_SECTOR_COUNT},
        {DAT_GRID_MODEL_0_3_SECTOR, DAT_GRID_MODEL_0_3_SECTOR_COUNT},
        {DAT_GRID_MODEL_0_4_SECTOR, DAT_GRID_MODEL_0_4_SECTOR_COUNT},
    },
    {
        {DAT_GRID_MODEL_1_0_SECTOR, DAT_GRID_MODEL_1_0_SECTOR_COUNT},
        {DAT_GRID_MODEL_1_1_SECTOR, DAT_GRID_MODEL_1_1_SECTOR_COUNT},
        {DAT_GRID_MODEL_1_2_SECTOR, DAT_GRID_MODEL_1_2_SECTOR_COUNT},
        {DAT_GRID_MODEL_1_3_SECTOR, DAT_GRID_MODEL_1_3_SECTOR_COUNT},
        {DAT_GRID_MODEL_1_4_SECTOR, DAT_GRID_MODEL_1_4_SECTOR_COUNT},
    },
    {
        {DAT_GRID_MODEL_2_0_SECTOR, DAT_GRID_MODEL_2_0_SECTOR_COUNT},
        {DAT_GRID_MODEL_2_1_SECTOR, DAT_GRID_MODEL_2_1_SECTOR_COUNT},
        {DAT_GRID_MODEL_2_2_SECTOR, DAT_GRID_MODEL_2_2_SECTOR_COUNT},
        {DAT_GRID_MODEL_2_3_SECTOR, DAT_GRID_MODEL_2_3_SECTOR_COUNT},
        {DAT_GRID_MODEL_2_4_SECTOR, DAT_GRID_MODEL_2_4_SECTOR_COUNT},
    },
    {
        {DAT_GRID_MODEL_3_0_SECTOR, DAT_GRID_MODEL_3_0_SECTOR_COUNT},
        {DAT_GRID_MODEL_3_1_SECTOR, DAT_GRID_MODEL_3_1_SECTOR_COUNT},
        {DAT_GRID_MODEL_3_2_SECTOR, DAT_GRID_MODEL_3_2_SECTOR_COUNT},
        {DAT_GRID_MODEL_3_3_SECTOR, DAT_GRID_MODEL_3_3_SECTOR_COUNT},
        {DAT_GRID_MODEL_3_4_SECTOR, DAT_GRID_MODEL_3_4_SECTOR_COUNT},
    },
    {
        {DAT_GRID_MODEL_4_0_SECTOR, DAT_GRID_MODEL_4_0_SECTOR_COUNT},
        {DAT_GRID_MODEL_4_1_SECTOR, DAT_GRID_MODEL_4_1_SECTOR_COUNT},
        {DAT_GRID_MODEL_4_2_SECTOR, DAT_GRID_MODEL_4_2_SECTOR_COUNT},
        {DAT_GRID_MODEL_4_3_SECTOR, DAT_GRID_MODEL_4_3_SECTOR_COUNT},
        {DAT_GRID_MODEL_4_4_SECTOR, DAT_GRID_MODEL_4_4_SECTOR_COUNT},
    },
    {
        {DAT_GRID_MODEL_5_0_SECTOR, DAT_GRID_MODEL_5_0_SECTOR_COUNT},
        {DAT_GRID_MODEL_5_1_SECTOR, DAT_GRID_MODEL_5_1_SECTOR_COUNT},
        {DAT_GRID_MODEL_5_2_SECTOR, DAT_GRID_MODEL_5_2_SECTOR_COUNT},
        {DAT_GRID_MODEL_5_3_SECTOR, DAT_GRID_MODEL_5_3_SECTOR_COUNT},
        {DAT_GRID_MODEL_5_4_SECTOR, DAT_GRID_MODEL_5_4_SECTOR_COUNT},
    },
    {
        {DAT_GRID_MODEL_6_0_SECTOR, DAT_GRID_MODEL_6_0_SECTOR_COUNT},
        {DAT_GRID_MODEL_6_1_SECTOR, DAT_GRID_MODEL_6_1_SECTOR_COUNT},
        {DAT_GRID_MODEL_6_2_SECTOR, DAT_GRID_MODEL_6_2_SECTOR_COUNT},
        {DAT_GRID_MODEL_6_3_SECTOR, DAT_GRID_MODEL_6_3_SECTOR_COUNT},
        {DAT_GRID_MODEL_6_4_SECTOR, DAT_GRID_MODEL_6_4_SECTOR_COUNT},
    },
    {
        {DAT_GRID_MODEL_7_0_SECTOR, DAT_GRID_MODEL_7_0_SECTOR_COUNT},
        {DAT_GRID_MODEL_7_1_SECTOR, DAT_GRID_MODEL_7_1_SECTOR_COUNT},
        {DAT_GRID_MODEL_7_2_SECTOR, DAT_GRID_MODEL_7_2_SECTOR_COUNT},
        {DAT_GRID_MODEL_7_3_SECTOR, DAT_GRID_MODEL_7_3_SECTOR_COUNT},
        {DAT_GRID_MODEL_7_4_SECTOR, DAT_GRID_MODEL_7_4_SECTOR_COUNT},
    },
    {
        {DAT_GRID_MODEL_8_0_SECTOR, DAT_GRID_MODEL_8_0_SECTOR_COUNT},
        {DAT_GRID_MODEL_8_1_SECTOR, DAT_GRID_MODEL_8_1_SECTOR_COUNT},
        {DAT_GRID_MODEL_8_2_SECTOR, DAT_GRID_MODEL_8_2_SECTOR_COUNT},
        {DAT_GRID_MODEL_8_3_SECTOR, DAT_GRID_MODEL_8_3_SECTOR_COUNT},
        {DAT_GRID_MODEL_8_4_SECTOR, DAT_GRID_MODEL_8_4_SECTOR_COUNT},
    },
    {
        {DAT_GRID_MODEL_9_0_SECTOR, DAT_GRID_MODEL_9_0_SECTOR_COUNT},
        {DAT_GRID_MODEL_9_1_SECTOR, DAT_GRID_MODEL_9_1_SECTOR_COUNT},
        {DAT_GRID_MODEL_9_2_SECTOR, DAT_GRID_MODEL_9_2_SECTOR_COUNT},
        {DAT_GRID_MODEL_9_3_SECTOR, DAT_GRID_MODEL_9_3_SECTOR_COUNT},
        {DAT_GRID_MODEL_9_4_SECTOR, DAT_GRID_MODEL_9_4_SECTOR_COUNT},
    }
};

POLY_F3 gNumberAdjustmentArrow __attribute__((section(".data.number_adjustment_arrow"))) = {
    4U << 24, 0, 0, 0, POLY_F3_CODE, 0, 0, 0, 0, 0, 0
};

void func_80093A54(void);

static inline RegisteredTaskEntry *getRegisteredPanelEntry(u32 resource, s32 index) {
    RegisteredTaskEntry *entries = (RegisteredTaskEntry *)&D_800D9504[resource];
    return &entries[index / 10];
}

void createRegisteredStaticTextPanelTask(s16 x, s16 y, s16 textX, s16 textY,
                                         const u16 *text, s32 mode, s32 depth, s32 index) {
    RegisteredTextPanelResource *resource;
    RegisteredTaskEntry *entry;
    createChildTaskWithReserve(drawRegisteredTextPanelTask);
    D_8013AE8C->resource14 = allocateTaskResource();
    resource = (RegisteredTextPanelResource *)&D_800D9504[D_8013AE8C->resource14];
    resource->index = index;
    resource->depth = depth;
    resource->mode = mode;
    resource->x = x;
    resource->y = y;
    resource->textX = textX;
    resource->textY = textY;
    resource->text = text;
    if (index % 10 == 0) {
        if (D_8013AE88->resource14 == 0xFFFF) { for (;;) {} }
        entry = getRegisteredPanelEntry(D_8013AE88->resource14, index);
    } else {
        if (D_8013AE88->resource16 == 0xFFFF) { for (;;) {} }
        entry = getRegisteredPanelEntry(D_8013AE88->resource16, index);
    }
    entry->command = 0;
    entry->flags = 0;
    entry->task = D_8013AE8C;
}

void drawRegisteredTextPanelTask(void) {
    RegisteredTextPanelResource *state;
    RegisteredTaskEntry *entry;
    if (D_8013AE88->resource14 == 0xFFFF) { for (;;) {} }
    state = (RegisteredTextPanelResource *)&D_800D9504[D_8013AE88->resource14];
    if (state->index % 10 == 0) {
        if (D_8013AE98->resource14 == 0xFFFF) { for (;;) {} }
        entry = getRegisteredPanelEntry(D_8013AE98->resource14, state->index);
    } else {
        if (D_8013AE98->resource16 == 0xFFFF) { for (;;) {} }
        entry = getRegisteredPanelEntry(D_8013AE98->resource16, state->index);
    }
    if (entry->task != D_8013AE88 || entry->command == 99) {
        deleteCurrentTaskSubtree();
    } else {
        setSpriteColor(128, 128, 128);
        func_8006EC84(state->textX, state->textY, 18, state->text, 7, -1,
                     &D_800EBFF0[D_8013AF78], state->depth);
        if (state->mode == 0) {
            func_8006EC84((s16)(state->textX + 32), state->textY, 18, &gRegisteredPanelMode0Label.glyph, 7, -1,
                         &D_800EBFF0[D_8013AF78], state->depth);
        } else if (state->mode == 1) {
            func_8006EC84((s16)(state->textX + 32), state->textY, 18, &gRegisteredPanelMode1Label.glyph, 7, -1,
                         &D_800EBFF0[D_8013AF78], state->depth);
        }
        setTexturedQuadColor(128, 128, 128);
        drawDefaultTexturedPanel(state->x, state->y, 152, 40, 5, 0,
                                &D_800EBFF0[D_8013AF78], state->depth);
    }
}

void createRegisteredDetailedTextPanelTask(s16 x, s16 y, s16 textX, s16 textY,
                                   const u16 *text, s32 value, s32 color, s32 mode,
                                   const u16 *content, s32 depth, s32 index) {
    RegisteredDetailedPanelResource *resource;
    RegisteredTaskEntry *entry;
    createChildTaskWithReserve(updateRegisteredDetailedTextPanelTask);
    D_8013AE8C->resource14 = allocateTaskResource();
    resource = (RegisteredDetailedPanelResource *)&D_800D9504[D_8013AE8C->resource14];
    resource->unk00 = 0;
    resource->unk02 = 0;
    resource->unk04 = 0;
    resource->unk06 = 0;
    resource->unk08 = 0;
    resource->x = x;
    resource->y = y;
    resource->textX = textX;
    resource->textY = textY;
    resource->text = text;
    resource->value = value;
    resource->color = color;
    resource->mode = mode;
    resource->content = content;
    resource->depth = depth;
    resource->index = index;
    D_8013AE8C->resource16 = allocateTaskResource();
    D_8013AE8C->data18 = 0;
    D_8013AE8C->data1A = 0;
    if (index % 10 == 0) {
        if (D_8013AE88->resource14 == 0xFFFF) { for (;;) {} }
        entry = getRegisteredPanelEntry(D_8013AE88->resource14, index);
    } else {
        if (D_8013AE88->resource16 == 0xFFFF) { for (;;) {} }
        entry = getRegisteredPanelEntry(D_8013AE88->resource16, index);
    }
    entry->command = 0;
    entry->flags = 0;
    entry->task = D_8013AE8C;
}

void updateRegisteredDetailedTextPanelTask(void) {
    RegisteredDetailedPanelResource *state;
    ValuePanelEntry *children;
    RegisteredTaskEntry *entry;
    s16 result;

    if (D_8013AE88->resource14 == 0xFFFF) { for (;;) {} }
    state = (RegisteredDetailedPanelResource *)&D_800D9504[D_8013AE88->resource14];
    if (D_8013AE88->resource16 == 0xFFFF) { for (;;) {} }
    children = (ValuePanelEntry *)&D_800D9504[D_8013AE88->resource16];
    if (state->index % 10 == 0) {
        if (D_8013AE98->resource14 == 0xFFFF) { for (;;) {} }
        entry = getRegisteredPanelEntry(D_8013AE98->resource14, state->index);
    } else {
        if (D_8013AE98->resource16 == 0xFFFF) { for (;;) {} }
        entry = getRegisteredPanelEntry(D_8013AE98->resource16, state->index);
    }

    if (entry->task == D_8013AE88 && entry->command != 99) {
        switch (D_8013AE88->data18) {
        case 0:
            createRegisteredStaticTextPanelTask(state->x, state->y, state->textX, state->textY,
                                                state->text, state->value, state->depth + 1, 1);
            if (state->mode == 0) {
                entry->flags = 2;
                D_8013AE88->data18 = 3;
            } else {
                createInputTimedSpriteTask(state->x + 132, state->y + 18,
                                           D_80123840, 2, (u8)state->color,
                                           0xB0, 0x40, state->depth, 11);
                D_8013AE88->data18 = 1;
            }
            break;
        case 1:
            result = children[1].active;
            if (result > 0) {
                break;
            }
            if (result < 0) {
                entry->flags = 1;
                D_8013AE88->data18 = 4;
            } else if (state->content == NULL) {
                entry->flags = 2;
                D_8013AE88->data18 = 4;
            } else {
                createRegisteredMovingPanelTask(state->color, (const s16 *)state->content,
                                                state->x + 28, state->y + 23, 0, 16,
                                                state->x + 28, state->y + 39,
                                                state->depth + 2, 21);
                D_8013AE88->data18 = 2;
            }
            break;
        case 2:
            result = children[2].active;
            if (result >= 0) {
                if (result == 0) {
                    entry->flags = 2;
                } else if (result == 1) {
                    entry->flags = 3;
                } else {
                    entry->flags = 4;
                }
                D_8013AE88->data18 = 4;
            }
            break;
        case 3:
            break;
        case 4:
            goto finish;
        }
    } else {
    finish:
        deleteCurrentTaskSubtree();
    }
}

void initializeCountedTaskResourcePair(void) {
    D_8013AE88->data18 = 0;
    D_8013AE98->data1C++;
    if (D_8013AE88->resource14 != 0xFFFF) {
        for (;;) {}
    }
    D_8013AE88->resource14 = allocateTaskResource();
    if (D_8013AE88->resource16 != 0xFFFF) {
        for (;;) {}
    }
    D_8013AE88->resource16 = allocateTaskResource();
    setCurrentTaskCallback(updateShopMenuTask);
    D_8013AE88->unk10 |= 2;
}

void updateShopMenuTask(void) {
    MoneyMenuSelection *selection;
    RegisteredTaskEntry *entries;
    s32 complete;
    s32 nextState;
    s16 selectedIndex;
    GameTask *parent;

    if (D_8013AE88->resource14 == 0xFFFF) {
        for (;;) {}
    }
    selection = (MoneyMenuSelection *)&D_800D9504[D_8013AE88->resource14];
    if (D_8013AE88->resource16 == 0xFFFF) {
        for (;;) {}
    }
    entries = (RegisteredTaskEntry *)&D_800D9504[D_8013AE88->resource16];

    switch (D_8013AE88->data18) {
    case 0: {
        s16 mode = 1;
        setMusicSequenceEnabled(mode);
        selection->mode = 0;
        if (shopMenuRememberedSelectionGp != 0) {
            selection->selectedIndex = shopMenuRememberedSelectionGp - 1;
        } else {
            selection->selectedIndex = shopMenuRememberedSelectionGp;
        }
        selection->previousIndex = selection->selectedIndex;
        selection->choiceResult = 0;
        createRegisteredSpacedChildTasks(mode, 0xB0, 0x40, (const u8 *)sShopMenuSprites, sShopMenuSpriteMapping,
                                         NULL, 0, 0, 20, -308, -96, 0, 24, 4, 176,
                                         drawSelectedTaskSprite, drawRegisteredTaskGridSpriteWithColor, 4, 1);
        if (gRacePlayers[0].unk009 == 8) {
            createRegisteredTextPanelTask(-320, 8, sShopMenuTexts, sShopMenuCharacter8TextMapping, 4, 11);
        } else if (gRacePlayers[0].unk009 == 9) {
            createRegisteredTextPanelTask(-320, 8, sShopMenuTexts, sShopMenuCharacter9TextMapping, 4, 11);
        } else {
            createRegisteredTextPanelTask(-320, 8, sShopMenuTexts, sShopMenuTextMapping, 4, 11);
        }
        createRegisteredCallbackTask(0, 172, -72, drawCurrentTaskMoneyPanel, 4, 21);
        createRegisteredTexturePairTask(172, -72, sShopMenuTextureMapping, 4, selection->selectedIndex, 1, 5, 31);
        D_8013AE88->data18 = 2;
        break;
    }
    case 2:
        if ((s16)entries[0].flags == 0 && (s16)entries[1].flags == 0 &&
            (s16)entries[2].flags == 0 && (s16)entries[3].flags == 0) {
            moveTaskChildrenWithSpacing(0, entries[0].task, -0x80000, -0x60000, 0x1E000, 0, 1);
            configureMovement2D((Movement2D *)&D_800D9504[entries[1].task->resource14],
                                -0x8C000, 0x8000, 0x1E000, 0, 7);
            configureMovement2D((Movement2D *)&D_800D9504[entries[2].task->resource14],
                                -0x8000, -0x48000, -0x1E000, 0, 7);
            configureMovement2D((Movement2D *)&D_800D9504[entries[3].task->resource14],
                                -0x8000, -0x48000, -0x1E000, 0, 7);
            D_8013AE88->data18 = 3;
        }
        break;
    case 3:
        if ((s16)entries[0].flags == 0 && (s16)entries[1].flags == 0 &&
            (s16)entries[2].flags == 0 && (s16)entries[3].flags == 0) {
            entries[0].command = 20;
            entries[1].command = 20;
            entries[2].command = 20;
            configureColorCycleTask(1, 0, 128, 8, 0);
            D_8013B41F = 0;
            D_8013AE88->data18 = 4;
        }
        break;
    case 4:
        complete = isTaskResourceTransitionComplete(0);
        if (complete == 1 && D_8013B41F == 0) {
            entries[0].command = 21;
            entries[1].command = 22;
            entries[2].command = 22;
            configureColorCycleTask(2, 64, 128, 4, 0);
            D_8013B41F = complete;
        }
        if (selection->choiceResult == 0 || (s16)entries[0].flags != 0) {
            break;
        }
        if (D_8013B41F == 0) {
            entries[0].command = 22;
            entries[1].command = 22;
            entries[2].command = 22;
            D_8013B41F = 1;
        }
        if (selection->choiceResult < 0) {
            GameTask *cancelParent = D_8013AE98;
            shopMenuRememberedSelectionGp = 0;
            cancelParent->data18 = 6;
            goto startExit;
        }
        selectedIndex = selection->selectedIndex;
        parent = D_8013AE98;
        nextState = sShopMenuParentStates[selectedIndex];
        parent->data18 = nextState;
        if ((gRacePlayers[0].unk009 == 8 || gRacePlayers[0].unk009 == 9) && nextState == 12) {
            entries[0].command = 21;
            selection->choiceResult = 0;
            break;
        }
        if (D_8013AE98->data18 == 6) {
            shopMenuRememberedSelectionGp = 0;
        } else {
            shopMenuRememberedSelectionGp = selection->selectedIndex + 1;
        }
        D_8013AE88->data18 = 8;
        break;
    case 8:
        entries[0].command = 10;
        D_8013AE88->data18 = 9;
        break;
    case 9:
        if ((s16)entries[0].flags != 0) {
            break;
        }
    startExit:
        D_8013AE88->data18 = 5;
        break;
    case 5:
        if ((s16)entries[0].flags == 0 && (s16)entries[1].flags == 0 &&
            (s16)entries[2].flags == 0 && (s16)entries[3].flags == 0) {
            queueMenuResourceReads((s16)D_8013AE98->data18);
            moveTaskChildrenWithSpacing(0, entries[0].task, -0x134000, -0x60000, 0x1E000, 0x1E000, -1000);
            entries[0].command = 23;
            configureMovement2D((Movement2D *)&D_800D9504[entries[1].task->resource14],
                                -0x140000, 0x8000, -0x1E000, 0x1E000, -1000);
            configureMovement2D((Movement2D *)&D_800D9504[entries[2].task->resource14],
                                0xAC000, -0x48000, 0x1E000, 0x1E000, -1000);
            configureMovement2D((Movement2D *)&D_800D9504[entries[3].task->resource14],
                                0xAC000, -0x48000, 0x1E000, 0x1E000, -1000);
            D_8013AE88->data18 = 6;
        }
        break;
    case 6:
        if ((s16)entries[0].flags == 0 && (s16)entries[1].flags == 0 &&
            (s16)entries[2].flags == 0 && (s16)entries[3].flags == 0) {
            D_8013AE88->data18 = 7;
        }
        break;
    default:
        D_8013AE98->data1C--;
        deleteCurrentTaskSubtree();
        break;
    }
}

/* Keep the array incomplete at the call site for absolute address scheduling. */
extern char gMoneyPanelCurrencyLabel[] __attribute__((section(".data")));

void drawCurrentTaskMoneyPanel(void) {
    Movement2D *position;

    if (D_8013AE88->resource14 == 0xFFFF) { for (;;) {} }
    position = (Movement2D *)&D_800D9504[D_8013AE88->resource14];
    if (D_8013AE88->data1A != 0) {
        func_8006C998(position->x / 4096 + 32, position->y / 4096 - 22,
                     0xB6, 0, -1, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
        func_8006C998(position->x / 4096 + 74, position->y / 4096 + 140,
                     0xBD, 10, -1, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
        drawDecimalNumber8x8(position->x / 4096 + 74, position->y / 4096 + 152,
                             6, 0, D_80130BF0[0], 0, -1, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
        drawWrappedText8x8(position->x / 4096 + 122, position->y / 4096 + 152,
                           2, gMoneyPanelCurrencyLabel, 0, -1, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
        if (D_8013AE88->data1A == 1) {
            func_8007240C(position->x / 4096 + 68, position->y / 4096 + 136, 68, 32,
                          D_8013B3A4, D_8013B3A5, D_8013B3A6, 2,
                          &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
        } else {
            func_8007240C(position->x / 4096 + 68, position->y / 4096 + 136, 68, 32,
                          128, 128, 128, 2, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
        }
    }
}

void initializeAlternateCountedTaskResourcePair(void) {
    D_8013AE88->data18 = 0;
    D_8013AE98->data1C++;
    if (D_8013AE88->resource14 != 0xFFFF) {
        for (;;) {}
    }
    D_8013AE88->resource14 = allocateTaskResource();
    if (D_8013AE88->resource16 != 0xFFFF) {
        for (;;) {}
    }
    D_8013AE88->resource16 = allocateTaskResource();
    setCurrentTaskCallback(func_8008DDC0);
    D_8013AE88->unk10 |= 2;
}

INCLUDE_ASM("asm/nonmatchings/menu/panel_tasks", func_8008DDC0);

void drawCurrentTaskSaveRecordMoneyPanelWithCharacterSprites(void) {
    Movement2D *position;
    u16 recordIndex;

    if (D_8013AE88->resource14 == 0xFFFF) { for (;;) {} }
    position = (Movement2D *)&D_800D9504[D_8013AE88->resource14];
    recordIndex = D_8013AE88->data18;
    if (D_8013AE88->data1A != 0) {
        func_8006C998(position->x / 4096 + 8, position->y / 4096 - 22,
                     0xB9, 0, -1, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
        func_8006C998(position->x / 4096 + 74, position->y / 4096 + 140,
                     0xBD, 10, -1, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
        {
            s32 amountX = (s16)(position->x / 4096 + 74);
            s32 amountY = (s16)(position->y / 4096 + 152);
            SaveRecord *records = (SaveRecord *)&D_8013080C;
            drawDecimalNumber8x8(amountX, amountY, 6, 0,
                                 ((GlobalFlagStatePrefix *)&records[(s16)recordIndex])->unk_3E4,
                                 0, -1, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
        }
        drawWrappedText8x8(position->x / 4096 + 122, position->y / 4096 + 152,
                           2, gMoneyPanelCurrencyLabel, 0, -1, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
        if (D_8013AE88->data1A == 1) {
            func_8007240C(position->x / 4096 + 68, position->y / 4096 + 136, 68, 32,
                          D_8013B3A4, D_8013B3A5, D_8013B3A6, 2,
                          &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
        } else {
            func_8007240C(position->x / 4096 + 68, position->y / 4096 + 136, 68, 32,
                          128, 128, 128, 2, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
        }
    }

    setSpriteColor(128, 128, 128);
    if (D_801317E4[gRacePlayers[0].unk009] != 0) {
        s32 offsetX;
        s32 sprite;

        if (gRacePlayers[0].unk009 == 5) {
            offsetX = 40;
            sprite = 0x208;
        } else {
            offsetX = 48;
            sprite = 0x169;
        }
        func_8006C998(offsetX + position->x / 4096, position->y / 4096,
                      sprite, 0, 0, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C + 2);
    }
    func_8006C998(position->x / 4096, position->y / 4096,
                  0x168, 0, -1, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C + 2);
}

void initializeCountedTaskResourcePairEC50(void) {
    D_8013AE88->data18 = 0;
    D_8013AE98->data1C++;
    if (D_8013AE88->resource14 != 0xFFFF) {
        for (;;) {}
    }
    D_8013AE88->resource14 = allocateTaskResource();
    if (D_8013AE88->resource16 != 0xFFFF) {
        for (;;) {}
    }
    D_8013AE88->resource16 = allocateTaskResource();
    setCurrentTaskCallback(updateMoneyPanelSelectionTask);
    D_8013AE88->unk10 |= 2;
}

void updateMoneyPanelSelectionTask(void) {
    MoneyMenuSelection *selection;
    RegisteredTaskEntry *entries;
    s32 complete;
    s16 mode;

    if (D_8013AE88->resource14 == 0xFFFF) {
        for (;;) {}
    }
    selection = (MoneyMenuSelection *)&D_800D9504[D_8013AE88->resource14];
    if (D_8013AE88->resource16 == 0xFFFF) {
        for (;;) {}
    }
    entries = (RegisteredTaskEntry *)&D_800D9504[D_8013AE88->resource16];

    switch (D_8013AE88->data18) {
    case 0:
        mode = 1;
        setMusicSequenceEnabled(mode);
        selection->mode = 0;
        if (D_8013B4BD != 0) {
            selection->selectedIndex = D_8013B4BD - 1;
        } else {
            selection->selectedIndex = 0;
        }
        selection->previousIndex = selection->selectedIndex;
        selection->choiceResult = 0;
        createRegisteredSpacedChildTasks(mode, 0xB0, 0x40, D_801317F0, sMoneyPanelSelectionSpriteMapping,
                                         NULL, 0, 0, 20, -308, -80, 0, 24, 3, 176,
                                         drawSelectedTaskSprite, drawRegisteredTaskGridSpriteWithColor, 4, 1);
        createRegisteredTextPanelTask(-320, 8, sShopMenuTexts, D_8013B4A0, 4, 11);
        createRegisteredCallbackTask(0, 172, -72, drawCurrentTaskMoneyPanelWithCharacterSprites, 4, 21);
        uploadTextureArchiveEntry(320, 0, 768, 0xE9, D_801317E4[gRacePlayers[0].unk009]);
        uploadTextureArchiveEntry(320, 256, 768, 0xEA, 0);
        createRegisteredTexturePairTask(172, -72, D_8013B4B4, 3, selection->selectedIndex, 1, 5, 31);
        D_8013AE88->data18 = 2;
        break;
    case 2:
        if ((s16)entries[0].flags == 0 && (s16)entries[1].flags == 0 &&
            (s16)entries[2].flags == 0 && (s16)entries[3].flags == 0) {
            moveTaskChildrenWithSpacing(0, entries[0].task, -0x80000, -0x50000, 0x1E000, 0, 1);
            configureMovement2D((Movement2D *)&D_800D9504[entries[1].task->resource14],
                                -0x8C000, 0x8000, 0x1E000, 0, 4);
            if (D_8013B4BD == 0) {
                setTaskSecondaryResourceData(entries[1].task, sShopMenuTexts[5]);
            }
            configureMovement2D((Movement2D *)&D_800D9504[entries[2].task->resource14],
                                -0x8000, -0x48000, -0x1E000, 0, 4);
            configureMovement2D((Movement2D *)&D_800D9504[entries[3].task->resource14],
                                -0x8000, -0x48000, -0x1E000, 0, 4);
            D_8013AE88->data18 = 3;
        }
        break;
    case 3:
        if ((s16)entries[0].flags == 0 && (s16)entries[1].flags == 0 &&
            (s16)entries[2].flags == 0 && (s16)entries[3].flags == 0) {
            entries[0].command = 20;
            entries[1].command = 20;
            entries[2].command = 20;
            configureColorCycleTask(1, 0, 128, 8, 0);
            D_8013B41F = 0;
            D_8013AE88->data18 = 4;
        }
        break;
    case 4:
        if (D_8013B4BD == 0) {
            playSoundEffect(0, 2, 32);
        }
        D_8013AE88->data18 = 5;
        break;
    case 5:
        complete = isTaskResourceTransitionComplete(0);
        if (complete == 1 && D_8013B41F == 0) {
            entries[0].command = 21;
            entries[1].command = 22;
            entries[2].command = 22;
            configureColorCycleTask(2, 64, 128, 4, 0);
            D_8013B41F = complete;
        }
        if (selection->selectedIndex != selection->previousIndex) {
            setTaskSecondaryResourceData(entries[1].task, NULL);
        }
        if (selection->choiceResult != 0 && (s16)entries[0].flags == 0) {
            if (D_8013B41F == 0) {
                entries[0].command = 22;
                entries[1].command = 22;
                entries[2].command = 22;
                D_8013B41F = 1;
            }
            if (selection->choiceResult < 0) {
                goto returnToParentMenu;
            }
            switch (selection->selectedIndex) {
            case 0:
                D_8013AE98->data18 = 18;
                break;
            case 1:
                D_8013AE98->data18 = 19;
                break;
            case 2:
returnToParentMenu:
                D_8013AE98->data18 = 11;
                break;
            }
            if (D_8013AE98->data18 == 11) {
                D_8013B4BD = 0;
            } else {
                D_8013B4BD = selection->selectedIndex + 1;
            }
            if (selection->choiceResult < 0) {
                goto finishChoice;
            }
            D_8013AE88->data18 = 9;
        }
        break;
    case 9:
        entries[0].command = 10;
        D_8013AE88->data18 = 10;
        break;
    case 10:
        if ((s16)entries[0].flags == 0) {
finishChoice:
            D_8013AE88->data18 = 11;
        }
        break;
    case 11:
        if (D_8013AE98->data18 == 11) {
            D_8013AE88->data18 = 12;
            break;
        }
        goto beginExit;
    case 12:
        playSoundEffect(0, 0x202, 32);
beginExit:
        D_8013AE88->data18 = 6;
        break;
    case 6:
        if ((s16)entries[0].flags == 0 && (s16)entries[1].flags == 0 &&
            (s16)entries[2].flags == 0 && (s16)entries[3].flags == 0) {
            queueMenuResourceReads((s16)D_8013AE98->data18);
            moveTaskChildrenWithSpacing(0, entries[0].task, -0x134000, -0x50000, 0x1E000, 0x1E000, -1000);
            entries[0].command = 23;
            configureMovement2D((Movement2D *)&D_800D9504[entries[1].task->resource14],
                                -0x140000, 0x8000, -0x1E000, 0x1E000, -1000);
            configureMovement2D((Movement2D *)&D_800D9504[entries[2].task->resource14],
                                0xAC000, -0x48000, 0x1E000, 0x1E000, -1000);
            configureMovement2D((Movement2D *)&D_800D9504[entries[3].task->resource14],
                                0xAC000, -0x48000, 0x1E000, 0x1E000, -1000);
            D_8013AE88->data18 = 7;
        }
        break;
    case 7:
        if ((s16)entries[0].flags == 0 && (s16)entries[1].flags == 0 &&
            (s16)entries[2].flags == 0 && (s16)entries[3].flags == 0) {
            D_8013AE88->data18 = 8;
        }
        break;
    default:
        D_8013AE98->data1C--;
        deleteCurrentTaskSubtree();
        break;
    }
}

void drawCurrentTaskMoneyPanelWithCharacterSprites(void) {
    Movement2D *position;

    if (D_8013AE88->resource14 == 0xFFFF) { for (;;) {} }
    position = (Movement2D *)&D_800D9504[D_8013AE88->resource14];
    if (D_8013AE88->data1A != 0) {
        func_8006C998(position->x / 4096 + 8, position->y / 4096 - 22,
                     0xB7, 0, -1, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
        func_8006C998(position->x / 4096 + 74, position->y / 4096 + 140,
                     0xBD, 10, -1, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
        drawDecimalNumber8x8(position->x / 4096 + 74, position->y / 4096 + 152,
                             6, 0, D_80130BF0[0], 0, -1, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
        drawWrappedText8x8(position->x / 4096 + 122, position->y / 4096 + 152,
                           2, gMoneyPanelCurrencyLabel, 0, -1, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
        if (D_8013AE88->data1A == 1) {
            func_8007240C(position->x / 4096 + 68, position->y / 4096 + 136, 68, 32,
                          D_8013B3A4, D_8013B3A5, D_8013B3A6, 2,
                          &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
        } else {
            func_8007240C(position->x / 4096 + 68, position->y / 4096 + 136, 68, 32,
                          128, 128, 128, 2, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
        }
    }

    setSpriteColor(128, 128, 128);
    if (D_801317E4[gRacePlayers[0].unk009] != 0) {
        s32 offsetX;
        s32 sprite;

        if (gRacePlayers[0].unk009 == 5) {
            offsetX = 40;
            sprite = 0x208;
        } else {
            offsetX = 48;
            sprite = 0x169;
        }
        func_8006C998(offsetX + position->x / 4096, position->y / 4096,
                      sprite, 0, -1, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C + 2);
    }
    func_8006C998(position->x / 4096, position->y / 4096,
                  0x168, 0, -1, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C + 2);
}

char gMoneyPanelCurrencyLabel[4] __attribute__((section(".data"), aligned(4))) = "\x82" "f";

void initializeCountedGroupedSelectionBoundsTask(void) {
    RECT *bounds;
    s16 selection;
    D_8013AE98->data1C++;
    D_8013AE88->data18 = 0;
    if (D_8013080C.flags & 0x1C000) {
        D_8013AE88->data1A = 1;
    } else {
        D_8013AE88->data1A = 0;
    }
    if (D_8013AE88->resource14 != 0xFFFF) { for (;;) {} }
    D_8013AE88->resource14 = allocateTaskResource();
    if (D_8013AE88->resource16 != 0xFFFF) { for (;;) {} }
    D_8013AE88->resource16 = allocateTaskResource();
    selection = getGroupedIndexBase(0);
    D_801300AC[0].selection = selection;
    D_801300AC[0].previousSelection = selection;
    D_801300B8[0].x = 0;
    D_801300B8[0].y = 0;
    D_801300B8[0].w = -1;
    D_801300B8[0].h = -1;
    D_801300B8[1].x = 0;
    bounds = &D_801300B8[1];
    bounds->y = 0;
    bounds->w = -1;
    bounds->h = -1;
    setCurrentTaskCallback(updateGroupedSelectionBoundsTask);
}

void updateGroupedSelectionBoundsTask(void) {
    FlagSelectedMenuState *input;
    RegisteredTaskEntry *entries;
    MenuSelectionState *selection;
    GroupedSelectionDescriptor *descriptor;
    s32 complete;

    if (D_8013AE88->resource14 == 0xFFFF) { for (;;) {} }
    input = (FlagSelectedMenuState *)&D_800D9504[D_8013AE88->resource14];
    if (D_8013AE88->resource16 == 0xFFFF) { for (;;) {} }
    entries = (RegisteredTaskEntry *)&D_800D9504[D_8013AE88->resource16];
    selection = &D_801300AC[0];

    switch (D_8013AE88->data18) {
    case 0: {
        GameTask *task;
        setMusicSequenceEnabled(1);
        descriptor = &D_801318D4[D_8013AE88->data1A];
        createRegisteredSpacedChildTasks(1, 0xB0, 0x40, D_80131868, descriptor->mapping,
                                        NULL, 0, 0, 20, -304, -96, descriptor->spacingX,
                                        descriptor->spacingY, descriptor->count, 0xB0,
                                        drawSelectedTaskSprite, drawRegisteredTaskGridSpriteWithColor, 4, 1);
        createRegisteredTextPanelTask(-316, 8, D_8012FFD4, descriptor->label, 4, 11);
        createRegisteredPanelTask(0, selection->selection, 244, 0, 384, 21);
        createRegisteredCallbackTask(0, 172, -72, drawCurrentTaskSaveRecordMoneyPanel, 4, 31);
        createRegisteredTripletTask(-296, 48, 0, 16, D_8012FFFC, descriptor->tripletIndices, 3, 41);
        task = D_8013AE88;
        input->mode = 0;
        input->selectedIndex = 0;
        input->previousIndex = 0;
        input->choiceResult = 0;
        task->data18 = 2;
        break;
    }
    case 2:
        if (CHILDREN_READY(entries)) {
            moveTaskChildrenWithSpacing(0, entries[0].task, -0x7C000, -0x60000, 0x1E000, 0x1E000, 1);
            configureMovement2D(CHILD_MOVEMENT(entries[1]), -0x88000, 0x8000, 0x1E000, 0x1E000, 5);
            func_80081AC8(0, entries[2].task, 0x40000, 0, 0x1E000, 0x1E000, 5);
            configureMovement2D(CHILD_MOVEMENT(entries[3]), -0x8000, -0x48000, 0x1E000, 0x1E000, 5);
            configureMovement2D(CHILD_MOVEMENT(entries[4]), -0x74000, 0x30000, 0x1E000, 0x1E000, 5);
            D_8013AE88->data18 = 3;
        }
        break;
    case 3:
        if (CHILDREN_READY(entries)) {
            GameTask *task;

            entries[0].command = 20;
            entries[1].command = 20;
            entries[2].command = 1;
            entries[3].command = 20;
            entries[4].command = 20;
            configureColorCycleTask(1, 0, 128, 8, 0);
            task = D_8013AE88;
            D_8013B41F = 0;
            task->data18 = 4;
        }
        break;
    case 4:
        complete = isTaskResourceTransitionComplete(0);
        if (complete == 1 && D_8013B41F == 0) {
            entries[0].command = 21;
            entries[1].command = 22;
            entries[3].command = 22;
            entries[4].command = 22;
            configureColorCycleTask(2, 64, 128, 4, 0);
            D_8013B41F = complete;
        }
        if (input->selectedIndex != input->previousIndex) {
            selection->previousSelection = selection->selection;
            selection->selection = getTaskResourceGroupedIndex(entries[0].task, input->selectedIndex);
            if (selection->selection == 9) {
                selection->selection += getSaveRecordFlagGroupIndex(0, 1);
            }
            if (selection->selection < 0) {
                selection->selection = selection->previousSelection;
            } else if (selection->selection != selection->previousSelection) {
                if (input->previousIndex < input->selectedIndex) {
                    entries[2].command = 11;
                } else {
                    entries[2].command = 12;
                }
            }
            input->previousIndex = input->selectedIndex;
        } else if (input->choiceResult != 0 && *(s16 *)&entries[0].flags == 0) {
            if (D_8013B41F == 0) {
                entries[0].command = 22;
                entries[1].command = 22;
                entries[3].command = 22;
                entries[4].command = 22;
                D_8013B41F = 1;
            }
            if (input->choiceResult < 0) {
                D_8013AE98->data18 = 12;
                goto startExit;
            }
            D_8013AE88->data18 = 13;
        }
        break;
    case 13: {
        GameTask *task = D_8013AE88;
        entries[0].command = 10;
        task->data18 = 14;
        break;
    }
    case 14:
        if (*(s16 *)&entries[0].flags == 0) {
            D_8013AE88->data18 = 5;
        }
        break;
    case 5: {
        GameTask *task = D_8013AE88;
        descriptor = &D_801318D4[task->data1A];
        if (descriptor->mapping[input->selectedIndex] == 17) {
            D_8013B4BC = 0;
            D_8013AE98->data18 = 11;
            playSoundEffect(0, 0x202, 32);
startExit:
            D_8013AE88->data18 = 6;
        } else {
            task->data18 = 9;
        }
        break;
    }
    case 9:
        createChildTaskWithReserve(initializeTaskResourcePair);
        D_8013AE8C->data18 = 0;
        descriptor = &D_801318D4[D_8013AE88->data1A];
        D_8013AE8C->data1A = func_8007E8CC(descriptor->mapping, input->selectedIndex);
        D_8013AE88->data18 = 10;
        break;
    case 11:
        D_8013AE98->data18 = 12;
        goto finishExit;
    case 6:
        if (CHILDREN_READY(entries)) {
            moveTaskChildrenWithSpacing(0, entries[0].task, -0x130000, -0x60000, 0x1E000, 0x1E000, -1000);
            entries[0].command = 23;
            configureMovement2D(CHILD_MOVEMENT(entries[1]), -0x13C000, 0x8000, 0x1E000, 0x1E000, -1000);
            func_80081AC8(0, entries[2].task, 0xF4000, 0, 0x1E000, 0, -1000);
            func_80081AC8(1, entries[2].task, 0xF4000, 0, 0x1E000, 0, -1000);
            configureMovement2D(CHILD_MOVEMENT(entries[3]), 0xAC000, -0x48000, 0x1E000, 0x1E000, -1000);
            configureMovement2D(CHILD_MOVEMENT(entries[4]), -0x128000, 0x30000, 0x1E000, 0x1E000, -1000);
            D_8013AE88->data18 = 7;
        }
        break;
    case 7:
        if (CHILDREN_READY(entries)) {
finishExit:
            D_8013AE88->data18 = 8;
        }
        break;
    case 8:
        D_8013AE88->data18 = 12;
        break;
    case 10:
        break;
    default:
        D_8013AE98->data1C--;
        deleteCurrentTaskSubtree();
        break;
    }
}

void initializeTaskResourcePair(void) {
    D_8013AE88->data18 = 0;
    if (D_8013AE88->resource14 != 0xFFFF) {
        for (;;) {}
    }
    D_8013AE88->resource14 = allocateTaskResource();
    if (D_8013AE88->resource16 != 0xFFFF) {
        for (;;) {}
    }
    D_8013AE88->resource16 = allocateTaskResource();
    setCurrentTaskCallback(func_80090450);
}

INCLUDE_ASM("asm/nonmatchings/menu/panel_tasks", func_80090450);

extern char gSaveRecordMoneyPanelCurrencyLabel[]
    __attribute__((section(".data.save_record_money_panel_currency_label")));

void drawCurrentTaskSaveRecordMoneyPanel(void) {
    Movement2D *position;
    u16 recordIndex;

    if (D_8013AE88->resource14 == 0xFFFF) { for (;;) {} }
    position = (Movement2D *)&D_800D9504[D_8013AE88->resource14];
    recordIndex = D_8013AE88->data18;
    if (D_8013AE88->data1A != 0) {
        func_8006C998(position->x / 4096 + 8, position->y / 4096 - 22,
                     0xB9, 0, -1, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
        func_8006C998(position->x / 4096 + 74, position->y / 4096 + 140,
                     0xBD, 10, -1, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
        {
            s32 amountX = (s16)(position->x / 4096 + 74);
            s32 amountY = (s16)(position->y / 4096 + 152);
            SaveRecord *records = (SaveRecord *)&D_8013080C;
            drawDecimalNumber8x8(amountX, amountY, 6, 0,
                                 ((GlobalFlagStatePrefix *)&records[(s16)recordIndex])->unk_3E4,
                                 0, -1, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
        }
        drawWrappedText8x8(position->x / 4096 + 122, position->y / 4096 + 152,
                           2, gSaveRecordMoneyPanelCurrencyLabel, 0, -1,
                           &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
        if (D_8013AE88->data1A == 1) {
            func_8007240C(position->x / 4096 + 68, position->y / 4096 + 136, 68, 32,
                          D_8013B3A4, D_8013B3A5, D_8013B3A6, 2,
                          &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
        } else {
            func_8007240C(position->x / 4096 + 68, position->y / 4096 + 136, 68, 32,
                          128, 128, 128, 2, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
        }
    }
}

char gSaveRecordMoneyPanelCurrencyLabel[4]
    __attribute__((section(".data.save_record_money_panel_currency_label"), aligned(4))) = "\x82" "f";

void drawCurrentTaskSelectedBoardDetails(void) {
    Movement2D *position;
    SelectionDisplayEntry *entry;

    if (D_8013AE88->resource14 == 0xFFFF) {
        for (;;) {}
    }
    position = (Movement2D *)&D_800D9504[D_8013AE88->resource14];
    entry = &((SelectionDisplayEntry *)&D_801300AC)[(s16)D_8013AE88->data18];
    if (D_8013AE88->data1A != 0) {
        s16 board = entry->index;
        u32 price = D_80131810[board];
        s32 priceOffset = 4;

        if (board < 9) {
            s32 textIndex = gBoardSelectionStyles[board];
            priceOffset = 0;
            drawWrappedDoubleByteText(position->x / 4096 - 104, position->y / 4096 + 8,
                10, gBoardStyleText[textIndex], 6, -1,
                &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
            drawWrappedDoubleByteText(position->x / 4096 - 104, position->y / 4096 + 24,
                10, (s8 *)gSelectedBoardLevelLabel, 6, -1,
                &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
            drawDecimalNumber8x16(position->x / 4096 - 64, position->y / 4096 + 24,
                1, 0, gBoardSelectionLevels[entry->index], 6, -1,
                &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
        }
        if (price == 0) {
            s32 x = (s16)(position->x / 4096 - 104);
            s32 y = position->y / 4096;
            s32 textOffset = priceOffset + 8;
            drawWrappedDoubleByteText(x, y + textOffset + 32,
                10, (s8 *)gSelectedBoardPricePlaceholder, 6, -1,
                &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
        } else {
            u32 remaining = price;
            s32 count = 0;
            s32 nextCount;
            s16 digits;
            s32 textOffset;

            do {
                nextCount = ++count;
                remaining /= 10;
            } while (remaining != 0);
            digits = nextCount;
            {
                s32 x = (s16)(position->x / 4096 - 104);
                s32 y = position->y / 4096;
                textOffset = priceOffset + 8;
                drawDecimalNumber8x16(x, y + textOffset + 32,
                    digits, 0, price, 6, -1, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
            }
            {
                s32 x = position->x / 4096;
                s32 currencyOffset = digits * 8 - 104;
                drawWrappedDoubleByteText(x + currencyOffset,
                    position->y / 4096 + textOffset + 32,
                    1, (s8 *)gSaveRecordMoneyPanelCurrencyLabel, 6, -1,
                    &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
            }
        }
    }
}

void drawCurrentTaskSelectedBoardPrice(void) {
    Movement2D *position;
    s16 player;
    SelectionDisplayEntry *entry;

    if (D_8013AE88->resource14 == 0xFFFF) {
        for (;;) {}
    }
    position = (Movement2D *)&D_800D9504[D_8013AE88->resource14];
    player = D_8013AE88->data18;
    entry = &((SelectionDisplayEntry *)&D_801300AC)[player];
    if (D_8013AE88->data1A != 0) {
        s16 board = entry->index;
        u8 flagIndex = D_80131804[board];
        u32 price = D_80131810[board];
        SaveRecord *records = (SaveRecord *)&D_8013080C;
        if (records[player].flags_3AC & (4U << flagIndex)) {
            func_8006C998(position->x / 4096 + 36, position->y / 4096 - 8, 0xC3, 15, -1,
                         &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
        } else {
            u32 remaining = price;
            s16 digits = 0;
            s32 xOffset;
            s32 x;

            while (remaining != 0) {
                remaining /= 10;
                digits++;
            }
            if (digits < 5) {
                digits = 5;
            }
            x = position->x / 4096;
            xOffset = digits * 4 - 64;
            drawDecimalNumber8x8(x - xOffset, position->y / 4096 - 4,
                                 digits, 0, price, 0, -1, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
            drawWrappedText8x8(position->x / 4096 - xOffset + digits * 8, position->y / 4096 - 4,
                               2, gSaveRecordMoneyPanelCurrencyLabel, 0, -1,
                               &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
            if (D_8013AE88->data1A == 1) {
                func_8007240C(position->x / 4096 - xOffset - 4, position->y / 4096 - 8,
                              digits * 8 + 16, 16, D_8013B3A4, D_8013B3A5, D_8013B3A6, 2,
                              &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
            } else {
                func_8007240C(position->x / 4096 - xOffset - 4, position->y / 4096 - 8,
                              digits * 8 + 16, 16, 128, 128, 128, 2,
                              &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
            }
        }
    }
}

void initializeCountedSelectionBoundsTask(void) {
    RECT *bounds;
    D_8013AE98->data1C++;
    D_8013AE88->data18 = 0;
    D_8013AE88->data1A = 0;
    if (D_8013AE88->resource14 != 0xFFFF) { for (;;) {} }
    D_8013AE88->resource14 = allocateTaskResource();
    if (D_8013AE88->resource16 != 0xFFFF) { for (;;) {} }
    D_8013AE88->resource16 = allocateTaskResource();
    D_801300AC[0].selection = 0;
    D_801300AC[0].previousSelection = 0;
    D_801300B8[0].x = 0;
    D_801300B8[0].y = 0;
    D_801300B8[0].w = -1;
    D_801300B8[0].h = -1;
    D_801300B8[1].x = 0;
    bounds = &D_801300B8[1];
    bounds->y = 0;
    bounds->w = -1;
    bounds->h = -1;
    setCurrentTaskCallback(updateBoardSelectionMenuTask);
}

void updateBoardSelectionMenuTask(void) {
    MoneyMenuSelection *selection;
    RegisteredTaskEntry *entries;
    MenuSelectionState *groupedSelection;
    GroupedSelectionDescriptor *descriptor;
    s32 complete;

    if (D_8013AE88->resource14 == 0xFFFF) { for (;;) {} }
    selection = (MoneyMenuSelection *)&D_800D9504[D_8013AE88->resource14];
    if (D_8013AE88->resource16 == 0xFFFF) { for (;;) {} }
    entries = (RegisteredTaskEntry *)&D_800D9504[D_8013AE88->resource16];
    groupedSelection = &D_801300AC[0];

    switch (D_8013AE88->data18) {
    case 0:
        setMusicSequenceEnabled(1);
        descriptor = &D_801318D4[D_8013AE88->data1A];
        createRegisteredSpacedChildTasks(1, 0xB0, 0x40, D_80131868,
                                         descriptor->mapping, NULL, 0, 0, 20, -304, -96,
                                         descriptor->spacingX, descriptor->spacingY,
                                         descriptor->count, 176,
                                         drawSelectedTaskSprite, drawRegisteredTaskGridSpriteWithColor, 4, 1);
        createRegisteredTextPanelTask(-316, 8, D_8012FFD4, descriptor->label, 4, 11);
        createRegisteredPanelTask(0, groupedSelection->selection, 244, 0, 384, 21);
        createRegisteredCallbackTask(0, 172, -72, drawCurrentTaskSpriteBA, 4, 31);
        createRegisteredTripletTask(-296, 48, 0, 16, D_8012FFFC, descriptor->tripletIndices, 3, 41);
        selection->mode = 0;
        selection->selectedIndex = 0;
        selection->previousIndex = 0;
        selection->choiceResult = 0;
        D_8013AE88->data18 = 2;
        break;
    case 2:
        if ((s16)entries[0].flags == 0 && (s16)entries[1].flags == 0 &&
            (s16)entries[2].flags == 0 && (s16)entries[3].flags == 0 &&
            (s16)entries[4].flags == 0) {
            moveTaskChildrenWithSpacing(0, entries[0].task, -0x7C000, -0x60000, 0x1E000, 0x1E000, 1);
            configureMovement2D((Movement2D *)&D_800D9504[entries[1].task->resource14],
                                -0x88000, 0x8000, 0x1E000, 0x1E000, 5);
            func_80081AC8(0, entries[2].task, 0x40000, 0, 0x1E000, 0x1E000, 5);
            configureMovement2D((Movement2D *)&D_800D9504[entries[3].task->resource14],
                                -0x8000, -0x48000, 0x1E000, 0x1E000, 5);
            configureMovement2D((Movement2D *)&D_800D9504[entries[4].task->resource14],
                                -0x74000, 0x30000, 0x1E000, 0x1E000, 5);
            D_8013AE88->data18 = 3;
        }
        break;
    case 3:
        if ((s16)entries[0].flags == 0 && (s16)entries[1].flags == 0 &&
            (s16)entries[2].flags == 0 && (s16)entries[3].flags == 0 &&
            (s16)entries[4].flags == 0) {
            entries[0].command = 20;
            entries[1].command = 20;
            entries[2].command = 1;
            entries[3].command = 20;
            entries[4].command = 20;
            configureColorCycleTask(1, 0, 128, 8, 0);
            D_8013B41F = 0;
            D_8013AE88->data18 = 4;
        }
        break;
    case 4:
        complete = isTaskResourceTransitionComplete(0);
        if (complete == 1 && D_8013B41F == 0) {
            entries[0].command = 21;
            entries[1].command = 22;
            entries[3].command = 22;
            entries[4].command = 22;
            configureColorCycleTask(2, 64, 128, 4, 0);
            D_8013B41F = complete;
        }
        if (selection->selectedIndex != selection->previousIndex) {
            groupedSelection->previousSelection = groupedSelection->selection;
            groupedSelection->selection = getTaskResourceGroupedIndex(entries[0].task, selection->selectedIndex);
            if (groupedSelection->selection < 0) {
                groupedSelection->selection = groupedSelection->previousSelection;
            } else if (groupedSelection->previousSelection != groupedSelection->selection) {
                if (selection->previousIndex >= selection->selectedIndex) {
                    entries[2].command = 12;
                } else {
                    entries[2].command = 11;
                }
            }
            selection->previousIndex = selection->selectedIndex;
        } else if (selection->choiceResult != 0 && (s16)entries[0].flags == 0) {
            if (D_8013B41F == 0) {
                entries[0].command = 22;
                entries[1].command = 22;
                entries[3].command = 22;
                entries[4].command = 22;
                D_8013B41F = 1;
            }
            if (selection->choiceResult < 0) {
                D_8013AE98->data18 = 12;
                goto beginExit;
            }
            D_8013AE88->data18 = 14;
        }
        break;
    case 14:
        entries[0].command = 10;
        D_8013AE88->data18 = 15;
        break;
    case 15:
        if ((s16)entries[0].flags == 0) {
            D_8013AE88->data18 = 5;
        }
        break;
    case 5:
        descriptor = &D_801318D4[D_8013AE88->data1A];
        if (descriptor->mapping[selection->selectedIndex] == 17) {
            D_8013B4BC = 0;
            D_8013AE98->data18 = 11;
            playSoundEffect(0, 0x202, 32);
beginExit:
            D_8013AE88->data18 = 6;
        } else {
            D_8013AE88->data18 = 9;
        }
        break;
    case 9:
        createChildTaskWithReserve(initializeTaskResourcesAndClearData18);
        D_8013AE8C->data18 = 0;
        descriptor = &D_801318D4[D_8013AE88->data1A];
        D_8013AE8C->data1A = func_8007E8CC(descriptor->mapping, selection->selectedIndex);
        D_8013AE88->data18 = 10;
        break;
    case 10:
        break;
    case 11:
        D_8013AE98->data18 = 12;
        goto finishExit;
    case 12:
        D_8013B4BC = 0;
        D_8013AE98->data18 = 11;
        playSoundEffect(0, 0x202, 32);
        D_8013AE88->data18 = 13;
        break;
    case 6:
        if ((s16)entries[0].flags == 0 && (s16)entries[1].flags == 0 &&
            (s16)entries[2].flags == 0 && (s16)entries[3].flags == 0 &&
            (s16)entries[4].flags == 0) {
            moveTaskChildrenWithSpacing(0, entries[0].task, -0x130000, -0x60000, 0x1E000, 0x1E000, -1000);
            entries[0].command = 23;
            configureMovement2D((Movement2D *)&D_800D9504[entries[1].task->resource14],
                                -0x13C000, 0x8000, 0x1E000, 0x1E000, -1000);
            func_80081AC8(0, entries[2].task, 0xF4000, 0, 0x1E000, 0, -1000);
            func_80081AC8(1, entries[2].task, 0xF4000, 0, 0x1E000, 0, -1000);
            configureMovement2D((Movement2D *)&D_800D9504[entries[3].task->resource14],
                                0xAC000, -0x48000, 0x1E000, 0x1E000, -1000);
            configureMovement2D((Movement2D *)&D_800D9504[entries[4].task->resource14],
                                -0x128000, 0x30000, 0x1E000, 0x1E000, -1000);
            D_8013AE88->data18 = 7;
        }
        break;
    case 7:
        if ((s16)entries[0].flags == 0 && (s16)entries[1].flags == 0 &&
            (s16)entries[2].flags == 0 && (s16)entries[3].flags == 0 &&
            (s16)entries[4].flags == 0) {
finishExit:
            D_8013AE88->data18 = 8;
        }
        break;
    default:
        D_8013AE98->data1C--;
        deleteCurrentTaskSubtree();
        break;
    }
}

void initializeTaskResourcesAndClearData18(void) {
    D_8013AE88->data18 = 0;
    if (D_8013AE88->resource14 != 0xFFFF) {
        while (1) {}
    }
    D_8013AE88->resource14 = allocateTaskResource();
    if (D_8013AE88->resource16 != 0xFFFF) {
        while (1) {}
    }
    D_8013AE88->resource16 = allocateTaskResource();
    setCurrentTaskCallback(updateBoardSelectionDetailsMenuTask);
}

void updateBoardSelectionDetailsMenuTask(void) {
    MoneyMenuSelection *parentSelection;
    RegisteredPanelSlot *parentPanels;
    MoneyMenuSelection *selection;
    RegisteredPanelSlot *panels;
    MenuSelectionState *groupedSelection;
    RecordFlagMenuEntries entryStorage;
    RecordFlagMenuEntries *entries;
    RegisteredPanelSlot *detailPanels;
    s32 complete;
    s16 value;

    if (D_8013AE98->resource14 == 0xFFFF) { for (;;) {} }
    parentSelection = (MoneyMenuSelection *)&D_800D9504[D_8013AE98->resource14];
    if (D_8013AE98->resource16 == 0xFFFF) { for (;;) {} }
    parentPanels = (RegisteredPanelSlot *)&D_800D9504[D_8013AE98->resource16];
    if (D_8013AE88->resource14 == 0xFFFF) { for (;;) {} }
    selection = (MoneyMenuSelection *)&D_800D9504[D_8013AE88->resource14];
    if (D_8013AE88->resource16 == 0xFFFF) { for (;;) {} }
    panels = (RegisteredPanelSlot *)&D_800D9504[D_8013AE88->resource16];
    groupedSelection = &D_801300AC[0];

    switch (D_8013AE88->data18) {
    case 0:
        moveTaskChildrenWithSpacing(2, parentPanels[0].task, -0x130000, -0x60000, 0x1E000, 0x1E000, -1000);
        D_8013AE88->data18 = 1;
        break;
    case 1:
        if (parentPanels[0].flags == 0) {
            parentPanels[0].command = 23;
            parentPanels[1].command = 23;
            parentPanels[4].command = 23;
            configureTaskGridMovement(1, parentPanels[0].task, -0x7C000, -0x60000, 0x1E000, 0x1E000, 0);
            D_8013AE88->data18 = 2;
        }
        break;
    case 2:
        if (parentPanels[0].flags == 0) {
            entries = &entryStorage;
            buildRecordFlagMenuEntries(0, D_8013AE98->data1A, parentSelection->selectedIndex, 0, 20, entries);
            createRegisteredSpacedChildTasks(1, 0xB0, 0x40, D_80131868,
                                             entries->entries, NULL, 0, 0, 20, -304, -96,
                                             entries->unk_04, entries->unk_05, entries->count, 222,
                                             drawSelectedTaskSprite, drawRegisteredTaskGridSpriteWithColor, 5, 1);
            createRegisteredTextPanelTask(-136, 8, NULL, entries->textIndices, 4, 11);
            createRegisteredCallbackTask(0, 0, 0, drawCurrentTaskBoardStyleAndLevel, 3, 21);
            selection->mode = 0;
            value = getGroupedIndexOffsetPlus216(groupedSelection->selection);
            selection->selectedIndex = value;
            value = findIndexedHalfwordRecord((IndexedHalfwordRecord *)D_80131868, entries->entries, entries->count, value);
            selection->selectedIndex = value;
            selection->previousIndex = value;
            selection->choiceResult = 0;
            D_8013AE88->data18 = 3;
        }
        break;
    case 3:
        if (panels[0].flags == 0 && panels[1].flags == 0 && panels[2].flags == 0) {
            initializeSpacedTaskGridPositions(0, panels[0].task, -0x74000, -0x60000, 0, 0);
            configureVerticalTaskGridMovement(0, panels[0].task, -0x74000, -0x4C000, 0x8000, 0x8000, 2);
            initMovement2D(TASK_POSITION(parentPanels[1].task), -0x13C000, 0x8000);
            goto showSelection;
        }
        break;
    case 4:
        if (panels[0].flags == 0 && parentPanels[1].flags == 0) {
            panels[0].command = 20;
            panels[1].command = 20;
            panels[2].command = 20;
            configureColorCycleTask(1, 0, 128, 8, 0);
            D_8013B41F = 0;
            D_8013AE88->data18 = 5;
        }
        break;
    case 5:
        complete = isTaskResourceTransitionComplete(0);
        if (complete == 1 && D_8013B41F == 0) {
            panels[0].command = 21;
            panels[1].command = 22;
            panels[2].command = 22;
            configureColorCycleTask(2, 64, 128, 4, 0);
            D_8013B41F = complete;
        }
        if (parentPanels[2].flags == 0) {
            if (selection->choiceResult > 0) {
                parentPanels[2].command = 2;
            }
            if (selection->choiceResult != 0) {
                D_8013AE88->data18 = 6;
            }
        }
        if (groupedSelection->selection == groupedSelection->previousSelection &&
            selection->selectedIndex != selection->previousIndex) {
            groupedSelection->previousSelection = groupedSelection->selection;
            groupedSelection->selection = getTaskResourceAdjustedGroupedIndex(D_8013AE88->data1A, panels[0].task, selection->selectedIndex);
            if (groupedSelection->selection < 0) {
                groupedSelection->selection = groupedSelection->previousSelection;
                selection->previousIndex = selection->selectedIndex;
            } else {
                parentPanels[2].command = 13;
                selection->previousIndex = selection->selectedIndex;
                setTaskSecondaryResourceData(panels[1].task, NULL);
            }
        } else if (D_8013AE88->data18 != 5 && D_8013B41F == 0) {
            panels[0].command = 22;
            panels[1].command = 22;
            panels[2].command = 22;
            D_8013B41F = 1;
        }
        break;
    case 6:
        if (selection->choiceResult != 0 && panels[0].flags == 0) {
            if (selection->choiceResult < 0) {
                D_8013AE88->data18 = 10;
            } else {
                D_8013AE88->data18 = 21;
            }
        }
        break;
    case 10:
        configureTaskGridMovement(0, panels[0].task, -0x74000, -0x60000, 0x1E000, 0x1E000, 0);
        panels[0].command = 23;
        D_8013AE88->data18 = 11;
        break;
    case 11:
        if (panels[0].flags == 0) {
            initializeSpacedTaskGridPositions(0, panels[0].task, -0x128000, -0x60000, 0, 0);
            moveTaskChildrenWithSpacing(1, parentPanels[0].task, -0x7C000, -0x60000, 0x1E000, 0x1E000, 0);
            D_8013AE88->data18 = 12;
        }
        break;
    case 12:
        if (parentPanels[0].flags == 0) {
            moveTaskChildrenWithSpacing(2, parentPanels[0].task, -0x7C000, -0x60000, 0x1E000, 0x1E000, 0);
            D_8013AE88->data18 = 13;
        }
        break;
    case 13:
        if (parentPanels[0].flags == 0) {
            initMovement2D(TASK_POSITION(panels[1].task), -0x13C000, 0x8000);
            initMovement2D(TASK_POSITION(parentPanels[1].task), -0x88000, 0x8000);
            parentPanels[2].command = 1;
            parentPanels[0].command = 20;
            parentPanels[1].command = 20;
            parentPanels[4].command = 20;
            configureColorCycleTask(1, 0, 128, 8, 0);
            D_8013B41F = 0;
            D_8013AE88->data18 = 14;
        }
        break;
    case 14:
        parentSelection->choiceResult = 0;
        D_8013AE98->data18 = 4;
        deleteCurrentTaskSubtree();
        break;
    case 21:
        panels[0].command = 10;
        D_8013AE88->data18 = 22;
        break;
    case 22:
        if (panels[0].flags == 0) {
            D_8013AE88->data18 = 15;
        }
        break;
    case 15:
        moveTaskChildrenWithSpacing(0, panels[0].task, -0x128000, -0x4C000, 0x1E000, 0x1E000, -1000);
        panels[0].command = 23;
        configureMovement2D(TASK_POSITION(panels[1].task), -0x13C000, 0x8000, 0x1E000, 0x1E000, -1000);
        configureMovement2D(TASK_POSITION(panels[2].task), -0xB4000, 0, 0x1E000, 0x1E000, -1000);
        configureTaskGridMovement(1, parentPanels[0].task, -0x130000, -0x60000, 0x1E000, 0x1E000, -1000);
        D_8013AE88->data18 = 16;
        break;
    case 16:
        if (panels[0].flags == 0 && panels[1].flags == 0 && panels[2].flags == 0 && parentPanels[0].flags == 0) {
            TaskResourceRecord *resources;
            createChildTaskWithReserve(initializePlayerTextureSelectionTask);
            D_8013AE8C->resource14 = allocateTaskResource();
            resources = D_800D9504;
            D_8013AE8C->resource16 = allocateTaskResource();
            detailPanels = (RegisteredPanelSlot *)&resources[D_8013AE8C->resource16];
            detailPanels[0].task = parentPanels[2].task;
            detailPanels[0].command = 0;
            detailPanels[0].flags = 0;
            detailPanels[1].task = parentPanels[3].task;
            detailPanels[1].command = 0;
            detailPanels[1].flags = 0;
            panels[4].task = D_8013AE8C;
            D_8013AE88->data18 = 17;
        }
        break;
    case 17:
        if (panels[4].task->data1E != 0) {
            parentPanels[2].command = panels[4].task->data1E;
            panels[4].task->data1E = 1;
        } else {
            panels[4].task->data1E = parentPanels[2].flags;
        }
        break;
    case 18:
        moveTaskChildrenWithSpacing(0, panels[0].task, -0x74000, -0x4C000, 0x1E000, 0x1E000, 0);
        panels[0].command = 23;
        configureMovement2D(TASK_POSITION(panels[1].task), -0x88000, 0x8000, 0x1E000, 0x1E000, 0);
        configureMovement2D(TASK_POSITION(panels[2].task), 0, 0, 0x1E000, 0, 0);
        configureTaskGridMovement(1, parentPanels[0].task, -0x7C000, -0x60000, 0x1E000, 0x1E000, 0);
        selection->choiceResult = 0;
        parentPanels[2].command = 1;
        D_8013AE88->data18 = 19;
        break;
    case 19:
        if (panels[0].flags == 0 && panels[1].flags == 0 && panels[2].flags == 0 && parentPanels[0].flags == 0) {
showSelection:
            D_8013AE88->data18 = 4;
        }
        break;
    case 20:
        D_8013AE98->data18 = 12;
        deleteCurrentTaskSubtree();
        break;
    case 7:
        moveTaskChildrenWithSpacing(0, panels[0].task, -0x128000, -0x4C000, 0x1E000, 0x1E000, -1000);
        panels[0].command = 23;
        configureMovement2D(TASK_POSITION(panels[1].task), -0x13C000, 0x8000, 0x1E000, 0x1E000, -1000);
        configureMovement2D(TASK_POSITION(panels[2].task), -0xB4000, 0, 0x1E000, 0x1E000, -1000);
        configureTaskGridMovement(0, parentPanels[0].task, -0x130000, -0x60000, 0x1E000, 0x1E000, -1000);
        configureMovement2D(TASK_POSITION(parentPanels[1].task), -0x13C000, 0x8000, 0x1E000, 0x1E000, -1000);
        func_80081AC8(0, parentPanels[2].task, 0xF4000, 0, 0x1E000, 0, -1000);
        func_80081AC8(1, parentPanels[2].task, 0xF4000, 0, 0x1E000, 0, -1000);
        configureMovement2D(TASK_POSITION(parentPanels[3].task), 0xAC000, -0x48000, 0x1E000, 0x1E000, -1000);
        configureMovement2D(TASK_POSITION(parentPanels[4].task), -0x128000, 0x30000, 0x1E000, 0x1E000, -1000);
        D_8013AE88->data18 = 8;
        break;
    case 8:
        if (panels[0].flags == 0 && panels[1].flags == 0 && panels[2].flags == 0 &&
            parentPanels[0].flags == 0 && parentPanels[1].flags == 0 && parentPanels[2].flags == 0 &&
            parentPanels[3].flags == 0 && parentPanels[4].flags == 0) {
            D_8013AE98->data18 = 11;
            deleteCurrentTaskSubtree();
        }
        break;
    default:
        D_8013AE88->data18 = 9;
        deleteCurrentTaskSubtree();
        break;
    }
}

void drawCurrentTaskBoardStyleAndLevel(void) {
    Movement2D *position;
    SelectionDisplayEntry *entry;
    s32 textIndex;
    if (D_8013AE88->resource14 == 0xFFFF) { for (;;) {} }
    position = (Movement2D *)&D_800D9504[D_8013AE88->resource14];
    entry = &((SelectionDisplayEntry *)&D_801300AC)[(s16)D_8013AE88->data18];
    if (D_8013AE88->data1A != 0) {
        textIndex = gBoardSelectionStyles[entry->index];
        drawWrappedDoubleByteText(position->x / 4096 - 104, position->y / 4096 + 8,
            10, gBoardStyleText[textIndex], 6, -1,
            &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
        drawWrappedDoubleByteText(position->x / 4096 - 104, position->y / 4096 + 24,
            10, (s8 *)gBoardLevelLabel, 6, -1, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
        drawDecimalNumber8x16(position->x / 4096 - 64, position->y / 4096 + 24,
            1, 0, gBoardSelectionLevels[entry->index], 6, -1,
            &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
    }
}

void initializePlayerTextureSelectionTask(void) {
    D_8013AE88->data18 = 0;
    D_8013AE88->data1E = 0;
    setCurrentTaskCallback(func_80093A54);
}

INCLUDE_ASM("asm/nonmatchings/menu/panel_tasks", func_80093A54);

void drawCurrentTaskSpriteBA(void) {
    Movement2D *position;
    if (D_8013AE88->resource14 == 0xFFFF) {
        for (;;) {}
    }
    position = (Movement2D *)&D_800D9504[D_8013AE88->resource14];
    if (D_8013AE88->data1A != 0) {
        func_8006C998(position->x / 4096 + 8, position->y / 4096 - 22,
                      0xBA, 0, -1, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
    }
}

void drawCurrentTaskFramedSpritePair16A(void) {
    Movement2D *position;
    if (D_8013AE88->resource14 == 0xFFFF) { for (;;) {} }
    position = (Movement2D *)&D_800D9504[D_8013AE88->resource14];
    if (D_8013AE88->data1A != 0) {
        func_8006C998(position->x / 4096, position->y / 4096,
                      0x16A, 0, -1, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
        func_8006C998(position->x / 4096 + 64, position->y / 4096,
                      0x16B, 0, -1, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
    }
    setTexturedQuadColor(128, 128, 128);
    drawTexturedFrameStyle9(position->x / 4096 - 4, position->y / 4096 - 4,
                           136, 36, 5, 1, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C - 1);
}

void initializeCountedDisplayBoundsTask(void) {
    RECT *bounds;
    RECT *view;
    RECT *last;
    D_8013AE98->data1C++;
    D_8013AE88->data18 = 0;
    D_8013AE88->data1A = 0;
    if (D_8013AE88->resource14 != 0xFFFF) { for (;;) {} }
    D_8013AE88->resource14 = allocateTaskResource();
    if (D_8013AE88->resource16 != 0xFFFF) { for (;;) {} }
    D_8013AE88->resource16 = allocateTaskResource();
    setSecondaryDisplayTaskMode(2);
    D_801300B8[0].x = 0;
    D_801300B8[0].y = 0;
    D_801300B8[0].w = -1;
    D_801300B8[0].h = -1;
    D_801300B8[1].x = 0;
    bounds = &D_801300B8[1];
    bounds->y = 0;
    bounds->w = -1;
    bounds->h = -1;
    D_801300B8[2].x = 24;
    view = &D_801300B8[2];
    view->y = 132;
    view->w = 120;
    view->h = 84;
    D_801300B8[3].x = 0;
    last = &D_801300B8[3];
    last->y = 0;
    last->w = -1;
    last->h = -1;
    setCurrentTaskCallback(updateBoardPurchaseMenuTask);
    D_8013AE88->unk10 |= 2;
}

void updateBoardPurchaseMenuTask(void) {
    FlagSelectedMenuState *input;
    RegisteredTaskEntry *entries;
    TaskPanelSelectionValue *selection;
    s16 panelValues[6];
    s32 result;
    u32 price;
    u16 player;
    s16 playerIndex;
    s16 character;
    s16 palette;

    if (D_8013AE88->resource14 == 0xFFFF) { for (;;) {} }
    input = (FlagSelectedMenuState *)&D_800D9504[D_8013AE88->resource14];
    if (D_8013AE88->resource16 == 0xFFFF) { for (;;) {} }
    entries = (RegisteredTaskEntry *)&D_800D9504[D_8013AE88->resource16];
    playerIndex = (s16)D_8013AE88->data1A;
    player = D_8013AE88->data1A;
    character = gRacePlayers[playerIndex].unk009;
    palette = gRacePlayers[playerIndex].unk00D;
    selection = &D_8013B508[playerIndex];

    switch (D_8013AE88->data18) {
    case 0: {
        s16 previewCharacter, previewPalette;
        s32 allOwned;
        s16 board;
        const u8 *mapping;
        s16 count;

        setMusicSequenceEnabled(1);
        setSecondaryDisplayTaskMode(0);
        input->selectedIndex = 0;
        input->previousIndex = 0;
        input->mode = 0;
        input->choiceResult = 0;
        selection->value = input->selectedIndex + 1;
        selection->unk02 = selection->value;
        allOwned = 1;
        board = 0;
        do {
            result = hasStateGridFlag((GlobalFlagStatePrefix *)&((SaveRecord *)&D_8013080C)[(s16)player], character, board);
            if (result != 1) {
                allOwned = 0;
            }
            board++;
        } while (board < 4);
        if (allOwned) {
            mapping = sBoardPurchaseFullSpriteMapping;
            count = 5;
        } else {
            mapping = sBoardPurchaseSpriteMapping;
            count = 4;
        }
        createRegisteredSpacedChildTasks(1, 0xB0, 0x40, (const u8 *)D_80131A78, mapping,
                                         NULL, 0, 0, 20, -304, -96, 0, 20, count, 0xB0,
                                         drawSelectedTaskSprite, drawRegisteredTaskGridSpriteWithColor, 4, 1);
        createRegisteredCallbackTask(0, 172, -72, drawCurrentTaskSelectedPlayerMoneyPanel, 4, 11);
        createRegisteredCallbackTask(0, 180, 0, drawCurrentTaskBoardPrice, 4, 21);
        previewCharacter = character;
        previewPalette = palette;
        createGridModelPreviewTask(0, previewCharacter, selection->value, previewPalette,
                                   244, gPaletteSelectionMainPreviewY[previewCharacter] + 50, 384, 41);
        createRegisteredCallbackTask(0, -180, 0, drawCurrentTaskFramedSprite20C, 4, 31);
        createGridModelPreviewTask(2, previewCharacter, gRacePlayers[(s16)player].unk00A[0], previewPalette,
                                   -260, gPaletteSelectionSidePreviewY[previewCharacter] + 118, 448, 51);
        D_8013AE88->data18 = 2;
        break;
    }
    case 2:
        if (BOARD_PURCHASE_CHILDREN_READY(entries) && (s16)entries[3].flags == 0 && (s16)entries[5].flags == 0) {
            moveTaskChildrenWithSpacing(0, entries[0].task, -0x7C000, -0x60000, 0x1E000, 0, 1);
            configureMovement2D(CHILD_MOVEMENT(entries[1]), -0x8000, -0x48000, 0x1E000, 0, 6);
            configureMovement2D(CHILD_MOVEMENT(entries[2]), 0, 0, 0x1E000, 0, 6);
            configureMovement2D(MODEL_PREVIEW_MOVEMENT(entries[4]), 0x40000,
                                (gPaletteSelectionMainPreviewY[character] + 50) * 4096, 0x1E000, 0, 6);
            configureMovement2D(CHILD_MOVEMENT(entries[3]), 0, 0, 0x1E000, 0x1E000, 6);
            D_8013AE88->data18 = 3;
        }
        break;
    case 3:
        if (BOARD_PURCHASE_CHILDREN_READY(entries) && (s16)entries[3].flags == 0) {
            configureMovement2D(MODEL_PREVIEW_MOVEMENT(entries[5]), -0x50000,
                                (gPaletteSelectionSidePreviewY[character] + 118) * 4096, 0x1E000, 0x1E000, 0);
            D_8013AE88->data18 = 4;
        }
        break;
    case 4:
        if ((s16)entries[5].flags == 0) {
            GameTask *task;
            entries[0].command = 20;
            entries[1].command = 20;
            entries[2].command = 20;
            entries[4].command = 1;
            configureColorCycleTask(1, 0, 128, 8, 0);
            task = D_8013AE88;
            D_8013B41F = 0;
            task->data18 = 5;
        }
        break;
    case 5: {
        result = isTaskResourceTransitionComplete(0);
        if (result == 1 && D_8013B41F == 0) {
            entries[0].command = 21;
            entries[1].command = 22;
            entries[2].command = 22;
            configureColorCycleTask(2, 64, 128, 4, 0);
            D_8013B41F = result;
        }
        if (input->selectedIndex != input->previousIndex) {
            selection->unk02 = selection->value;
            if ((u16)func_80073760(entries[0].task, input->selectedIndex) < 5U) {
                selection->value = input->selectedIndex + 1;
                if (selection->value != selection->unk02) {
                    entries[4].command = 13;
                }
            }
            input->previousIndex = input->selectedIndex;
        } else if (input->choiceResult != 0 && (s16)entries[0].flags == 0) {
            if (D_8013B41F == 0) {
                entries[0].command = 22;
                entries[1].command = 22;
                entries[2].command = 22;
                D_8013B41F = 1;
            }
            if (input->choiceResult < 0) {
                D_8013AE98->data18 = 13;
                goto leave;
            }
            if (func_80073760(entries[0].task, input->selectedIndex) == 5) {
                boardMenuRememberedSelectionAbsolute = 0;
                D_8013AE98->data18 = 11;
                playSoundEffect(0, 0x202, 32);
                D_8013AE88->data18 = 15;
            } else {
                D_8013AE88->data18 = 11;
            }
        }
        break;
    }
    case 15: {
        GameTask *task = D_8013AE88;
        entries[0].command = 10;
        task->data18 = 16;
        break;
    }
    case 16:
        if ((s16)entries[0].flags == 0) {
leave:
            D_8013AE88->data18 = 6;
        }
        break;
    case 11: {
        s16 recordIndex = player;
        GlobalFlagStatePrefix *record = (GlobalFlagStatePrefix *)&((SaveRecord *)&D_8013080C)[recordIndex];
        s16 boardCharacter = character;
        if (hasStateGridFlag(record, boardCharacter, selection->value) == 1) {
            D_8013AE88->data18 = 17;
        } else if ((price = D_801319B0[boardCharacter * 5 + selection->value]) > record->unk_3E4) {
            playSoundEffect(0, 0x302, 32);
            D_8013AE88->data18 = 17;
        } else {
            createRegisteredCallbackTask(recordIndex, 0, -28, drawCurrentTaskTextPanel, 2, 101);
            panelValues[0] = 0x125;
            panelValues[2] = 0x126;
            panelValues[4] = 0xB0;
            panelValues[1] = 0;
            panelValues[3] = 0;
            panelValues[5] = 0;
            createRegisteredMovingPanelTask(1, panelValues, 24, -13, 0, 16, 24, 19, 3, 111);
            playSoundEffect(0, 0x502, 32);
            D_8013AE88->data18 = 12;
        }
        break;
    }
    case 12:
        if ((s16)entries[11].flags >= 0) {
            entries[10].command = 99;
            if ((s16)entries[11].flags != 0) {
                D_8013AE88->data18 = 17;
            } else {
                price = D_801319B0[character * 5 + selection->value];
                entries[12].task = createValueTransitionTask(
                    &((GlobalFlagStatePrefix *)&((SaveRecord *)&D_8013080C)[(s16)player])->unk_3E4,
                    ((GlobalFlagStatePrefix *)&((SaveRecord *)&D_8013080C)[(s16)player])->unk_3E4 - price, 100);
                D_8013AE88->data18 = 13;
            }
        }
        break;
    case 13: {
        GameTask *transition;
        if (gFrameCounter & 1) {
            playSoundEffect(1, 2, 32);
        }
        transition = entries[12].task;
        if (transition->data1A == 1) {
            s16 recordIndex;
            transition->data18 = 1;
            playSoundEffect(0, 0x102, 32);
            recordIndex = player;
            setStateGridFlag((GlobalFlagStatePrefix *)&((SaveRecord *)&D_8013080C)[recordIndex], character, selection->value);
            createRegisteredCallbackTask(recordIndex, 0, -28, drawCurrentTaskSelectionPanel, 2, 101);
            entries[12].task = createCountdownTask(15);
            D_8013AE88->data18 = 14;
        }
        break;
    }
    case 14: {
        GameTask *countdown = entries[12].task;
        if (countdown->data1A == 1) {
            GameTask *task;
            countdown->data18 = 1;
            task = D_8013AE88;
            entries[10].command = 99;
            task->data18 = 17;
        }
        break;
    }
    case 17: {
        GameTask *task;
        input->choiceResult = 0;
        entries[0].command = 21;
        configureColorCycleTask(2, 64, 128, 4, 0);
        task = D_8013AE88;
        D_8013B41F = 1;
        task->data18 = 5;
        break;
    }
    case 6:
        if (BOARD_PURCHASE_CHILDREN_READY(entries)) {
            GameTask *task;
            queueMenuResourceReads((s16)D_8013AE98->data18);
            configureMovement2D(MODEL_PREVIEW_MOVEMENT(entries[5]), -0x104000,
                                (gPaletteSelectionSidePreviewY[character] + 118) * 4096, 0x1E000, 0x1E000, -1000);
            task = D_8013AE88;
            entries[3].command = 23;
            task->data18 = 7;
        }
        break;
    case 7:
        if ((s16)entries[5].flags == 0) {
            moveTaskChildrenWithSpacing(0, entries[0].task, -0x130000, -0x60000, 0x1E000, 0x1E000, -1000);
            entries[0].command = 23;
            configureMovement2D(CHILD_MOVEMENT(entries[1]), 0xAC000, -0x48000, 0x1E000, 0x1E000, -1000);
            configureMovement2D(CHILD_MOVEMENT(entries[2]), 0xB4000, 0, 0x1E000, 0x1E000, -1000);
            configureMovement2D(MODEL_PREVIEW_MOVEMENT(entries[4]), 0xF4000,
                                (gPaletteSelectionMainPreviewY[character] + 50) * 4096, 0x1E000, 0x1E000, -1000);
            configureMovement2D(CHILD_MOVEMENT(entries[3]), -0xB4000, 0, 0x1E000, 0x1E000, -1000);
            D_8013AE88->data18 = 8;
        }
        break;
    case 8:
        if (BOARD_PURCHASE_CHILDREN_READY(entries) && (s16)entries[3].flags == 0) {
            D_8013AE88->data18 = 9;
        }
        break;
    case 9:
        D_8013AE88->data18 = 10;
        break;
    default:
        D_8013AE98->data1C--;
        deleteCurrentTaskSubtree();
        break;
    }
}

void drawCurrentTaskSelectedPlayerMoneyPanel(void) {
    Movement2D *position;
    u16 recordIndex;

    if (D_8013AE88->resource14 == 0xFFFF) { for (;;) {} }
    position = (Movement2D *)&D_800D9504[D_8013AE88->resource14];
    recordIndex = D_8013AE98->data1A;
    if (D_8013AE88->data1A != 0) {
        func_8006C998(position->x / 4096 + 8, position->y / 4096 - 22,
                     0xB7, 0, -1, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
        func_8006C998(position->x / 4096 + 74, position->y / 4096 + 140,
                     0xBD, 10, -1, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
        {
            s32 amountX = (s16)(position->x / 4096 + 74);
            s32 amountY = (s16)(position->y / 4096 + 152);
            SaveRecord *records = (SaveRecord *)&D_8013080C;
            drawDecimalNumber8x8(amountX, amountY, 6, 0,
                                 ((GlobalFlagStatePrefix *)&records[(s16)recordIndex])->unk_3E4,
                                 0, -1, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
        }
        drawWrappedText8x8(position->x / 4096 + 122, position->y / 4096 + 152,
                           2, D_8013B4FC, 0, -1,
                           &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
        if (D_8013AE88->data1A == 1) {
            func_8007240C(position->x / 4096 + 68, position->y / 4096 + 136, 68, 32,
                          D_8013B3A4, D_8013B3A5, D_8013B3A6, 2,
                          &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
        } else {
            func_8007240C(position->x / 4096 + 68, position->y / 4096 + 136, 68, 32,
                          128, 128, 128, 2, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
        }
    }
}

void drawCurrentTaskBoardPrice(void) {
    Movement2D *position;
    s16 player;
    s16 character;
    TaskPanelSelectionValue *selection;
    s32 x, y, fixedY;
    u8 offset;
    s16 priceX, priceY;
    u32 price, remaining;
    s16 digits;
    u32 halfWidth;

    if (D_8013AE88->resource14 == 0xFFFF) {
        for (;;) {}
    }
    position = (Movement2D *)&D_800D9504[D_8013AE88->resource14];
    player = D_8013AE98->data1A;
    character = gRacePlayers[player].unk009;
    selection = &D_8013B508[player];
    x = position->x / 4096;
    priceX = x;
    fixedY = position->y;
    if (fixedY < 0) {
        fixedY += 4095;
    }
    offset = sBoardPriceVerticalOffsets[character];
    fixedY >>= 12;
    y = (s8)offset + fixedY;
    priceY = y;

    if (D_8013AE88->data1A != 0) {
        SaveRecord *record = (SaveRecord *)&D_8013080C + player;
        s16 column = selection->value;

        price = D_801319B0[character * 5 + column];
        if (hasStateGridFlag((GlobalFlagStatePrefix *)record, character, column) == 1) {
            func_8006C998(x + 36, y + 8, 0xC3, 15, -1,
                          &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
        } else {
            remaining = price;
            digits = 0;
            while (remaining != 0) {
                remaining /= 10;
                digits++;
            }
            if (digits < 5) {
                digits = 5;
            }
            halfWidth = digits * 4;
            priceX -= halfWidth - 64;
            drawDecimalNumber8x8(priceX, priceY + 12, digits, 0, price, 0, -1,
                                 &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
            drawWrappedText8x8(priceX + digits * 8, priceY + 12, 2, D_8013B4FC, 0, -1,
                               &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
            if (D_8013AE88->data1A == 1) {
                func_8007240C(priceX - 4, priceY + 8, digits * 8 + 16, 16,
                              D_8013B3A4, D_8013B3A5, D_8013B3A6, 2,
                              &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
            } else {
                func_8007240C(priceX - 4, priceY + 8, digits * 8 + 16, 16, 128, 128, 128, 2,
                              &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
            }
        }
    }
}

s32 hasStateGridFlag(GlobalFlagStatePrefix *state, s16 row, s16 column) {
    if (row < 5) {
        if (state->unk_3DC & (1 << (row * 5 + column))) {
            return 1;
        }
    } else if (row < 10) {
        if (state->unk_3E0 & (1 << ((row - 5) * 5 + column))) {
            return 1;
        }
    }
    return 0;
}

void setStateGridFlag(GlobalFlagStatePrefix *state, s16 row, s16 column) {
    if (row < 5) {
        state->unk_3DC |= 1 << (row * 5 + column);
    } else if (row < 10) {
        state->unk_3E0 |= 1 << ((row - 5) * 5 + column);
    }
}

void drawCurrentTaskFramedSprite20C(void) {
    Movement2D *position;
    if (D_8013AE88->resource14 == 0xFFFF) { for (;;) {} }
    position = (Movement2D *)&D_800D9504[D_8013AE88->resource14];
    setTexturedQuadColor(128, 128, 128);
    setSpriteColor(128, 128, 128);
    drawTexturedFrameStyle9(position->x / 4096 - 140, position->y / 4096 + 8,
                           128, 92, 5, 1, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
    func_8006C998(position->x / 4096 - 136, position->y / 4096 + 12,
                  0x20C, 1, 0, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
}

void drawCurrentTaskTextPanel(void) {
    Movement2D *position;
    s32 x, y;
    if (D_8013AE88->resource14 == 0xFFFF) { for (;;) {} }
    position = (Movement2D *)&D_800D9504[D_8013AE88->resource14];
    x = position->x / 4096;
    y = position->y / 4096;
    setSpriteColor(128, 128, 128);
    setTexturedQuadColor(128, 128, 128);
    func_8006EF30((s16)(x + 16), (s16)(y + 12), 10, gCurrentTaskPanelText,
                 8, -1, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
    drawDefaultTexturedPanel(x, y, 144, 48, 5, 0,
                            &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
}

void drawCurrentTaskSelectionPanel(void) {
    Movement2D *position;
    s32 x, y;
    TaskPanelSelectionValue *selection;
    if (D_8013AE88->resource14 == 0xFFFF) { for (;;) {} }
    position = (Movement2D *)&D_800D9504[D_8013AE88->resource14];
    x = position->x / 4096;
    y = position->y / 4096;
    selection = &D_8013B508[(s16)D_8013AE98->data1A];
    setSpriteColor(128, 128, 128);
    setTexturedQuadColor(128, 128, 128);
    func_8006EF30((s16)(x + 8), (s16)(y + 4), 10, gTaskSelectionPanelLabel,
                 8, -1, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
    drawDecimalNumber16((s16)(x + 56), (s16)(y + 4), 1, 0, selection->value + 1,
                 8, -1, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
    func_8006EF30((s16)(x + 8), (s16)(y + 20), 10, gTaskSelectionPanelText,
                 8, -1, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
    drawDefaultTexturedPanel(x, y, 144, 48, 5, 0,
                            &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
}

void queueGridResourceSectorReads(s16 slot, s16 row, s16 column) {
    if (row < 10) {
        queueCdSectorCommand(0, gGridResourceFirstSectorRanges[row][column].sector,
                            gGridResourceFirstSectorRanges[row][column].sectorCount,
                            DAT_GRID_MODEL_BUFFERS + slot * 0x4000, 0);
        queueCdSectorCommand(0, gGridResourceSecondSectorRanges[row][column].sector,
                            gGridResourceSecondSectorRanges[row][column].sectorCount,
                            DAT_GRID_TEXTURE_BUFFERS + slot * 0x4000, 0);
    } else {
        queueCdSectorCommand(0, DAT_GRID_FALLBACK_SECTOR, DAT_GRID_FALLBACK_SECTOR_COUNT, DAT_GRID_MODEL_BUFFERS + slot * 0x4000, 0);
    }
}

void createGridModelPreviewTask(s16 playerIndex, s16 character, s16 modelSlot, s16 paletteRow,
                                s16 x, s16 y, s16 z, s32 index) {
    GridModelPreviewState *state;
    Movement2D *position;
    RegisteredTaskEntry *entry;
    u32 *source;
    u32 *destination;
    u32 *model;
    s32 i;
    u32 (*sourceBuffers)[4096];
    u32 (*destinationBuffers)[4096];
    RacePlayer *attributePlayer;
    RacePlayer *positionPlayer;

    createChildTaskWithReserve(updateGridModelPreviewTask);
    i = 0;
    D_8013AE8C->data1A = modelSlot;
    D_8013AE8C->data1C = playerIndex;
    D_8013AE8C->data1E = index;
    D_8013AE8C->resource14 = allocateTaskResource();
    sourceBuffers = (u32 (*)[4096])DAT_GRID_MODEL_BUFFERS;
    destinationBuffers = DAT_PLAYER_MODEL_BUFFERS;
    currentPlayerAbsolute = &gRacePlayers[D_8013AE8C->data1C];
    state = (GridModelPreviewState *)&D_800D9504[D_8013AE8C->resource14];
    currentPlayerAbsolute->characterId = character;
    currentPlayerAbsolute->unk015[0] = modelSlot;
    currentPlayerAbsolute->unk015[3] = paletteRow;

    destination = destinationBuffers[D_8013AE8C->data1C];
    source = sourceBuffers[D_8013AE8C->data1A];
    for (; i < 4096; i++) {
        *destination++ = *source++;
    }

    model = DAT_PLAYER_MODEL_BUFFERS[D_8013AE8C->data1C];
    model++;
    GsMapModelingData(model);
    model++;
    currentPlayerAbsolute->modelPartCount = *(u8 *)model;
    model++;
    if (D_8013AE8C->data1C != 0) {
        offsetTmdObjectTextures(0, (TmdObject *)model, &gPlayerRootTextureOffsets[D_8013AE8C->data1C]);
        for (i = 1; i < currentPlayerAbsolute->modelPartCount; i++) {
            offsetTmdObjectTextures(i, (TmdObject *)model, &gPlayerPartTextureOffsets[D_8013AE8C->data1C]);
        }
    }

    GsInitCoordinate2(0, &currentPlayerAbsolute->modelCoordinate);
    for (i = 0; i < currentPlayerAbsolute->modelPartCount; i++) {
        GsDOBJ2 *object;

        GsLinkObject4(model, &currentPlayerAbsolute->modelObjects[i], i);
        object = &currentPlayerAbsolute->modelObjects[i];
        GsInitCoordinate2(&currentPlayerAbsolute->modelCoordinate, &currentPlayerAbsolute->model.modelCoordinates[i]);
        object->attribute = 0;
        object->coord2 = &currentPlayerAbsolute->model.modelCoordinates[i];
    }

    attributePlayer = currentPlayerAbsolute;
    if (attributePlayer->modelPartCount >= 2) {
        attributePlayer->modelObjects[0].attribute |= GS_MODEL_OBJECT_HIDDEN;
    }
    uploadTimImageWithOffset(((u32 (*)[4096])DAT_GRID_TEXTURE_BUFFERS)[D_8013AE8C->data1A],
                             (s16 *)&D_801204D4[D_8013AE8C->data1C]);

    positionPlayer = currentPlayerAbsolute;
    positionPlayer->modelCoordinate.coord.t[0] = x;
    positionPlayer->modelCoordinate.coord.t[1] = y;
    positionPlayer->modelCoordinate.coord.t[2] = z;
    positionPlayer->unk0B4 = 0;
    positionPlayer->unk0B6 = 0;
    positionPlayer->unk0B8 = 0;
    state->rotationStep = 0;
    if (character < 10) {
        loadCurrentPlayerModelPose(gGridModelPreviewPoses[character]);
    }

    D_8013AE8C->resource16 = allocateTaskResource();
    position = (Movement2D *)&D_800D9504[D_8013AE8C->resource16];
    initMovement2D(position, x << 12, y << 12);
    position->unk08 = z;

    if (index % 10 == 0) {
        if (D_8013AE88->resource14 == 0xFFFF) { for (;;) {} }
        entry = getRegisteredPanelEntry(D_8013AE88->resource14, index);
    } else {
        if (D_8013AE88->resource16 == 0xFFFF) { for (;;) {} }
        entry = getRegisteredPanelEntry(D_8013AE88->resource16, index);
    }
    entry->command = 0;
    entry->flags = 1;
    entry->task = D_8013AE8C;
    copyPlayerPaletteRowWithTransition(playerIndex, character, paletteRow, 0);
}

void reloadGridModelPreview(GameTask *task, s16 character, s16 modelSlot, s16 paletteRow,
                            s16 x, s16 y, s16 z) {
    GridModelPreviewState *state = (GridModelPreviewState *)&D_800D9504[task->resource14];
    s16 playerIndex = task->data1C;
    GridModelPreviewState saved;
    SVECTOR rotation;
    u32 *source;
    u32 *destination;
    u32 *model;
    s32 i;
    RacePlayer *rotationPlayer;
    RacePlayer *attributePlayer;
    RacePlayer *positionPlayer;

    task->data1A = modelSlot;
    saved = *state;
    currentPlayerAbsolute = &gRacePlayers[task->data1C];
    currentPlayerAbsolute->characterId = character;
    currentPlayerAbsolute->unk015[0] = modelSlot;
    currentPlayerAbsolute->unk015[3] = paletteRow;
    rotationPlayer = currentPlayerAbsolute;
    rotation.vx = rotationPlayer->unk0B4;
    rotation.vy = rotationPlayer->unk0B6;
    rotation.vz = rotationPlayer->unk0B8;

    {
        u32 (*sourceBuffers)[4096] = (u32 (*)[4096])DAT_GRID_MODEL_BUFFERS;
        u32 (*destinationBuffers)[4096] = DAT_PLAYER_MODEL_BUFFERS;

        destination = destinationBuffers[task->data1C];
        source = sourceBuffers[task->data1A];
    }
    for (i = 0; i < 4096; i++) {
        *destination++ = *source++;
    }

    model = DAT_PLAYER_MODEL_BUFFERS[task->data1C];
    model++;
    GsMapModelingData(model);
    model++;
    currentPlayerAbsolute->modelPartCount = *(u8 *)model;
    model++;
    if (task->data1C != 0) {
        offsetTmdObjectTextures(0, (TmdObject *)model, &gPlayerRootTextureOffsets[task->data1C]);
        for (i = 1; i < currentPlayerAbsolute->modelPartCount; i++) {
            offsetTmdObjectTextures(i, (TmdObject *)model, &gPlayerPartTextureOffsets[task->data1C]);
        }
    }

    GsInitCoordinate2(0, &currentPlayerAbsolute->modelCoordinate);
    for (i = 0; i < currentPlayerAbsolute->modelPartCount; i++) {
        GsDOBJ2 *object;

        GsLinkObject4(model, &currentPlayerAbsolute->modelObjects[i], i);
        object = &currentPlayerAbsolute->modelObjects[i];
        GsInitCoordinate2(&currentPlayerAbsolute->modelCoordinate, &currentPlayerAbsolute->model.modelCoordinates[i]);
        object->attribute = 0;
        object->coord2 = &currentPlayerAbsolute->model.modelCoordinates[i];
    }

    attributePlayer = currentPlayerAbsolute;
    if (attributePlayer->modelPartCount >= 2) {
        attributePlayer->modelObjects[0].attribute |= GS_MODEL_OBJECT_HIDDEN;
    }
    uploadTimImageWithOffset(((u32 (*)[4096])DAT_GRID_TEXTURE_BUFFERS)[task->data1A],
                             (s16 *)&D_801204D4[task->data1C]);

    positionPlayer = currentPlayerAbsolute;
    positionPlayer->modelCoordinate.coord.t[0] = x;
    positionPlayer->modelCoordinate.coord.t[1] = y;
    positionPlayer->modelCoordinate.coord.t[2] = z;
    positionPlayer->unk0B4 = rotation.vx;
    positionPlayer->unk0B6 = rotation.vy;
    positionPlayer->unk0B8 = rotation.vz;
    state->rotationStep = saved.rotationStep;
    if (character < 10) {
        loadCurrentPlayerModelPose(gGridModelPreviewPoses[character]);
    }
    initMovement2D((Movement2D *)&D_800D9504[task->resource16], x << 12, y << 12);
    copyPlayerPaletteRowWithTransition(playerIndex, character, paletteRow, 0);
}

void updateGridModelPreviewTask(void) {
    u8 padding[0x10];
    GridModelPreviewState *state;
    Movement2D *position;
    TaskPanelSelectionValue *selection;
    RegisteredTaskEntry *entry;

    currentPlayerAbsolute = &gRacePlayers[D_8013AE88->data1C];
    selection = &D_8013B508[(s16)D_8013AE98->data1A];
    if (D_8013AE88->resource14 == 0xFFFF) { for (;;) {} }
    state = (GridModelPreviewState *)&D_800D9504[D_8013AE88->resource14];
    if (D_8013AE88->resource16 == 0xFFFF) { for (;;) {} }
    position = (Movement2D *)&D_800D9504[D_8013AE88->resource16];
    if ((u16)(D_8013AE88->data1E % 10U) == 0) {
        if (D_8013AE98->resource14 == 0xFFFF) { for (;;) {} }
        entry = getModelPreviewEntry(D_8013AE98->resource14, D_8013AE88->data1E);
    } else {
        if (D_8013AE98->resource16 == 0xFFFF) { for (;;) {} }
        entry = getModelPreviewEntry(D_8013AE98->resource16, D_8013AE88->data1E);
    }

    switch (entry->command) {
    case 1:
        state->rotationStep = 1;
        entry->command = 0;
        entry->flags = 0;
        break;
    case 2:
        state->rotationStep = -32;
        entry->command = 0;
        entry->flags = 0;
        break;
    case 3:
        state->rotationStep = 0;
        entry->command = 0;
        entry->flags = 0;
        break;
    case 0:
        entry->flags = 0;
        break;
    case 13:
        reloadGridModelPreview(D_8013AE88, gRacePlayers[D_8013AE88->data1C].characterId,
                               selection->value, gRacePlayers[D_8013AE88->data1C].unk015[3],
                               position->x / 4096, position->y / 4096, position->unk08);
        entry->command = 0;
        entry->flags = 0;
        selection->unk02 = selection->value;
        break;
    case 99:
    default:
        entry->task = 0;
        entry->flags = 0;
        deleteCurrentTaskSubtree();
        return;
    }

    entry->flags |= updateMovement2D(position);
    currentPlayerAbsolute->modelCoordinate.coord.t[0] = position->x / 4096;
    currentPlayerAbsolute->modelCoordinate.coord.t[1] = position->y / 4096;
    currentPlayerAbsolute->modelCoordinate.coord.t[2] = position->unk08;
    if (state->rotationStep > 0) {
        currentPlayerAbsolute->unk0B6 = (-32 * gFrameCounter) & 0xFFF;
    } else {
        currentPlayerAbsolute->unk0B6 = 0x480;
    }
    setCoordinateRotation((SVECTOR *)&currentPlayerAbsolute->unk0B4, &currentPlayerAbsolute->modelCoordinate);
    sortPlayerModelWithDrawArea(D_8013AE88->data1C, D_8013AE88->data1C, &D_801300B8[D_8013AE88->data1C]);
}
