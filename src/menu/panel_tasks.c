#include "panel_tasks_internal.h"

u8 gBoardSelectionLevels[12] __attribute__((section(".data.board_selection_levels"))) = {
    1, 1, 1, 2, 2, 2, 3, 3, 3, 1, 2, 3
};

s8 *gBoardStyleText[4] __attribute__((section(".data.board_style_text"))) = {
    D_80011DC8, D_80011DC8 + 0x10, D_80011DC8 + 0x20, D_80011DC8 + 0x2C
};

u8 gBoardSelectionStyles[12] __attribute__((section(".data.board_selection_styles"))) = {
    0, 1, 2, 0, 1, 2, 0, 1, 2, 3, 3, 3
};

const u8 gBoardLevelLabel[16] __attribute__((section(".rodata.board_level_label"), aligned(4))) = {
    0x82, 0x6B, 0x82, 0x64, 0x82, 0x75, 0x82, 0x64,
    0x82, 0x6B, 0, 0, 0, 0, 0, 0
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

INCLUDE_ASM("asm/nonmatchings/menu/panel_tasks", func_8008C708);

INCLUDE_ASM("asm/nonmatchings/menu/panel_tasks", func_8008C8BC);

static inline RegisteredTaskEntry *getRegisteredPanelEntry(u32 resource, s32 index) {
    RegisteredTaskEntry *entries = (RegisteredTaskEntry *)&D_800D9504[resource];
    return &entries[index / 10];
}

void createRegisteredDetailedTextPanelTask(s16 x, s16 y, s16 textX, s16 textY,
                                   const u16 *text, s32 value, s32 color, s32 mode,
                                   const u16 *content, s32 depth, s32 index) {
    RegisteredDetailedPanelResource *resource;
    RegisteredTaskEntry *entry;
    func_80014874(func_8008CD98);
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

INCLUDE_ASM("asm/nonmatchings/menu/panel_tasks", func_8008CD98);

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

INCLUDE_ASM("asm/nonmatchings/menu/panel_tasks", func_8008D958);

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

INCLUDE_ASM("asm/nonmatchings/menu/panel_tasks", func_8008E66C);

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
    setCurrentTaskCallback(func_8008EC50);
    D_8013AE88->unk10 |= 2;
}

INCLUDE_ASM("asm/nonmatchings/menu/panel_tasks", func_8008EC50);

INCLUDE_ASM("asm/nonmatchings/menu/panel_tasks", func_8008F474);

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
    setCurrentTaskCallback(func_8008FA8C);
}

INCLUDE_ASM("asm/nonmatchings/menu/panel_tasks", func_8008FA8C);

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

INCLUDE_ASM("asm/nonmatchings/menu/panel_tasks", func_80091318);

INCLUDE_ASM("asm/nonmatchings/menu/panel_tasks", func_800916F4);

INCLUDE_ASM("asm/nonmatchings/menu/panel_tasks", func_80091B80);

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
    setCurrentTaskCallback(func_80092090);
}

INCLUDE_ASM("asm/nonmatchings/menu/panel_tasks", func_80092090);

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
        func_8006D984(position->x / 4096 - 64, position->y / 4096 + 24,
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

INCLUDE_ASM("asm/nonmatchings/menu/panel_tasks", func_80095674);

INCLUDE_ASM("asm/nonmatchings/menu/panel_tasks", func_80095A58);

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
    func_8006E2C4((s16)(x + 56), (s16)(y + 4), 1, 0, selection->value + 1,
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

INCLUDE_ASM("asm/nonmatchings/menu/panel_tasks", func_80096590);

INCLUDE_ASM("asm/nonmatchings/menu/panel_tasks", func_80096AB0);

INCLUDE_ASM("asm/nonmatchings/menu/panel_tasks", func_80096EC8);
