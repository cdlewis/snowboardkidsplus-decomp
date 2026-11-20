#include "panel_tasks_internal.h"
#include "game/menu/task_widgets.h"
#include "game/menu/selected_sprite.h"
#include "game/engine/game_task_scheduler.h"

#define GS_MODEL_OBJECT_HIDDEN 0x80000000u

#define CHILDREN_READY(entries) (!*(s16 *)&(entries)[0].flags && !*(s16 *)&(entries)[1].flags && \
                                 !*(s16 *)&(entries)[2].flags && !*(s16 *)&(entries)[3].flags && \
                                 !*(s16 *)&(entries)[4].flags)
#define CHILD_MOVEMENT(entry) ((Movement2D *)&D_800D9504[(entry).task->resource14])


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

typedef struct MoneyMenuText {
    const u16 *labels[5];
    const u16 *initialDescription;
} MoneyMenuText;

extern const u8 D_80131868[];
extern GroupedSelectionDescriptor D_801318D4[];
extern ByteTriplet D_8012FFFC[];
extern u8 D_8013B4BC __attribute__((section(".data")));
extern MoneyMenuText D_80131794;
extern const u8 D_801317F0[];
extern const u8 D_8013B4A0[];
extern const u8 D_8013B4B4[];
extern const u8 D_8013B4B8[];
extern u8 D_8013B4BD;

extern const s16 D_80131A9C[], D_80131B08[], D_80131B74[];
extern const s16 D_80131BE0[], D_80131C4C[], D_80131CB8[];

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

extern void func_8008C708(s16 x, s16 y, s16 textX, s16 textY,
                         const u16 *text, s32 mode, s32 depth, s32 index);

INCLUDE_ASM("asm/nonmatchings/menu/panel_tasks", func_8008C708);

static inline RegisteredTaskEntry *getRegisteredPanelEntry(u32 resource, s32 index) {
    RegisteredTaskEntry *entries = (RegisteredTaskEntry *)&D_800D9504[resource];
    return &entries[index / 10];
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
        func_800149A8();
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
    func_80014874(updateRegisteredDetailedTextPanelTask);
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
            func_8008C708(state->x, state->y, state->textX, state->textY,
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
        func_800149A8();
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
    setCurrentTaskCallback(func_8008D200);
    D_8013AE88->unk10 |= 2;
}

INCLUDE_ASM("asm/nonmatchings/menu/panel_tasks", func_8008D200);

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
        createRegisteredSpacedChildTasks(mode, 0xB0, 0x40, D_801317F0, D_8013B4B8,
                                         NULL, 0, 0, 20, -308, -80, 0, 24, 3, 176,
                                         drawSelectedTaskSprite, drawRegisteredTaskGridSpriteWithColor, 4, 1);
        createRegisteredTextPanelTask(-320, 8, D_80131794.labels, D_8013B4A0, 4, 11);
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
                setTaskSecondaryResourceData(entries[1].task, D_80131794.initialDescription);
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
        func_800149A8();
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
        func_80014874(initializeTaskResourcePair);
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
        func_800149A8();
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
        func_80014874(initializeTaskResourcesAndClearData18);
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
        func_800149A8();
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
    setCurrentTaskCallback(func_80092A50);
}

INCLUDE_ASM("asm/nonmatchings/menu/panel_tasks", func_80092A50);

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

INCLUDE_ASM("asm/nonmatchings/menu/panel_tasks", func_80093A20);

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
    setCurrentTaskCallback(func_8009488C);
    D_8013AE88->unk10 |= 2;
}

INCLUDE_ASM("asm/nonmatchings/menu/panel_tasks", func_8009488C);

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

    func_80014874(updateGridModelPreviewTask);
    i = 0;
    D_8013AE8C->data1A = modelSlot;
    D_8013AE8C->data1C = playerIndex;
    D_8013AE8C->data1E = index;
    D_8013AE8C->resource14 = allocateTaskResource();
    sourceBuffers = (u32 (*)[4096])DAT_GRID_MODEL_BUFFERS;
    destinationBuffers = DAT_PLAYER_MODEL_BUFFERS;
    D_8013B0D8 = &gRacePlayers[D_8013AE8C->data1C];
    state = (GridModelPreviewState *)&D_800D9504[D_8013AE8C->resource14];
    D_8013B0D8->characterId = character;
    D_8013B0D8->unk015[0] = modelSlot;
    D_8013B0D8->unk015[3] = paletteRow;

    destination = destinationBuffers[D_8013AE8C->data1C];
    source = sourceBuffers[D_8013AE8C->data1A];
    for (; i < 4096; i++) {
        *destination++ = *source++;
    }

    model = DAT_PLAYER_MODEL_BUFFERS[D_8013AE8C->data1C];
    model++;
    GsMapModelingData(model);
    model++;
    D_8013B0D8->modelPartCount = *(u8 *)model;
    model++;
    if (D_8013AE8C->data1C != 0) {
        offsetTmdObjectTextures(0, (TmdObject *)model, &gPlayerRootTextureOffsets[D_8013AE8C->data1C]);
        for (i = 1; i < D_8013B0D8->modelPartCount; i++) {
            offsetTmdObjectTextures(i, (TmdObject *)model, &gPlayerPartTextureOffsets[D_8013AE8C->data1C]);
        }
    }

    GsInitCoordinate2(0, &D_8013B0D8->modelCoordinate);
    for (i = 0; i < D_8013B0D8->modelPartCount; i++) {
        GsDOBJ2 *object;

        GsLinkObject4(model, &D_8013B0D8->modelObjects[i], i);
        object = &D_8013B0D8->modelObjects[i];
        GsInitCoordinate2(&D_8013B0D8->modelCoordinate, &D_8013B0D8->model.modelCoordinates[i]);
        object->attribute = 0;
        object->coord2 = &D_8013B0D8->model.modelCoordinates[i];
    }

    attributePlayer = D_8013B0D8;
    if (attributePlayer->modelPartCount >= 2) {
        attributePlayer->modelObjects[0].attribute |= GS_MODEL_OBJECT_HIDDEN;
    }
    uploadTimImageWithOffset(((u32 (*)[4096])DAT_GRID_TEXTURE_BUFFERS)[D_8013AE8C->data1A],
                             (s16 *)&D_801204D4[D_8013AE8C->data1C]);

    positionPlayer = D_8013B0D8;
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
    D_8013B0D8 = &gRacePlayers[task->data1C];
    D_8013B0D8->characterId = character;
    D_8013B0D8->unk015[0] = modelSlot;
    D_8013B0D8->unk015[3] = paletteRow;
    rotationPlayer = D_8013B0D8;
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
    D_8013B0D8->modelPartCount = *(u8 *)model;
    model++;
    if (task->data1C != 0) {
        offsetTmdObjectTextures(0, (TmdObject *)model, &gPlayerRootTextureOffsets[task->data1C]);
        for (i = 1; i < D_8013B0D8->modelPartCount; i++) {
            offsetTmdObjectTextures(i, (TmdObject *)model, &gPlayerPartTextureOffsets[task->data1C]);
        }
    }

    GsInitCoordinate2(0, &D_8013B0D8->modelCoordinate);
    for (i = 0; i < D_8013B0D8->modelPartCount; i++) {
        GsDOBJ2 *object;

        GsLinkObject4(model, &D_8013B0D8->modelObjects[i], i);
        object = &D_8013B0D8->modelObjects[i];
        GsInitCoordinate2(&D_8013B0D8->modelCoordinate, &D_8013B0D8->model.modelCoordinates[i]);
        object->attribute = 0;
        object->coord2 = &D_8013B0D8->model.modelCoordinates[i];
    }

    attributePlayer = D_8013B0D8;
    if (attributePlayer->modelPartCount >= 2) {
        attributePlayer->modelObjects[0].attribute |= GS_MODEL_OBJECT_HIDDEN;
    }
    uploadTimImageWithOffset(((u32 (*)[4096])DAT_GRID_TEXTURE_BUFFERS)[task->data1A],
                             (s16 *)&D_801204D4[task->data1C]);

    positionPlayer = D_8013B0D8;
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

    D_8013B0D8 = &gRacePlayers[D_8013AE88->data1C];
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
        func_800149A8();
        return;
    }

    entry->flags |= updateMovement2D(position);
    D_8013B0D8->modelCoordinate.coord.t[0] = position->x / 4096;
    D_8013B0D8->modelCoordinate.coord.t[1] = position->y / 4096;
    D_8013B0D8->modelCoordinate.coord.t[2] = position->unk08;
    if (state->rotationStep > 0) {
        D_8013B0D8->unk0B6 = (-32 * gFrameCounter) & 0xFFF;
    } else {
        D_8013B0D8->unk0B6 = 0x480;
    }
    func_8001E3E4((SVECTOR *)&D_8013B0D8->unk0B4, &D_8013B0D8->modelCoordinate);
    sortPlayerModelWithDrawArea(D_8013AE88->data1C, D_8013AE88->data1C, &D_801300B8[D_8013AE88->data1C]);
}
