#define TASK_WIDGET_COLORS_ABSOLUTE_ADDRESSING
#include "psx/scratchpad.h"
#include "psx/libetc.h"
#include "../menu/save_record_tasks_internal.h"
#include "game/engine/byte_search.h"
#include "game/io/cd_queue.h"
#include "game/menu/selected_sprite.h"
#include "game/menu/task_widgets.h"

#define MEMORY_CARD_SLOT_BUFFER_BASE ((u8 *)0x801E4000)
#define MEMORY_CARD_SLOT_BUFFER_STRIDE 0x2000

typedef struct MenuModelResource {
    GsDOBJ2 object;
    GsCOORDINATE2 local;
    SVECTOR localRotation;
    GsCOORDINATE2 world;
    SVECTOR worldRotation;
    s16 unkC0, unkC2, unkC4, unkC6;
} MenuModelResource;

typedef struct MenuPlayerModelPosition {
    s32 x;
    s32 y;
    s16 z;
} MenuPlayerModelPosition;

typedef struct ClippedGridSpriteParent {
    u8 unk00[0x44];
    const GridSpriteEntry *sprites;
    const u8 *mapping;
    u8 unk4C[8];
    u16 mode;
    u8 unk56[4];
    u16 count;
    s8 alternateSprites;
} ClippedGridSpriteParent;

typedef struct SaveRecordTaskStatus {
    RegisteredPanelSlot first;
    u8 unk08[8];
    s16 secondResult;
} SaveRecordTaskStatus;

typedef struct SelectionItemStats {
    u8 rating;
    u16 price;
} SelectionItemStats;

u16 litObjectDrawAreaDepths[4] __attribute__((section(".data.lit_object_draw_area_depths"))) = {
    0x30, 0x20, 0x50, 0x3E
};

HalfwordPair D_8012F13C[16] __attribute__((section(".data"))) = {0};

TexturedQuadListEntry taskIndexPanelQuadsPalette0[10] __attribute__((section(".data.task_index_panel_quads"))) = {
    {33, 0, 0, 0, 0, 0},
    {34, 0, 72, 0, 176, 0},
    {35, 0, 248, 0, 0, 0},
    {36, 0, 0, 16, 0, 72},
    {37, 0, 72, 16, 176, 72},
    {38, 0, 248, 16, 0, 72},
    {39, 0, 0, 88, 0, 0},
    {40, 0, 72, 88, 176, 0},
    {41, 0, 248, 88, 0, 0},
    {0, 255, 0, 0, 0, 0},
};

TexturedQuadListEntry taskIndexPanelQuadsPalette1[10] __attribute__((section(".data.task_index_panel_quads"))) = {
    {33, 1, 0, 0, 0, 0},
    {34, 1, 72, 0, 176, 0},
    {35, 1, 248, 0, 0, 0},
    {36, 1, 0, 16, 0, 72},
    {37, 1, 72, 16, 176, 72},
    {38, 1, 248, 16, 0, 72},
    {39, 1, 0, 88, 0, 0},
    {40, 1, 72, 88, 176, 0},
    {41, 1, 248, 88, 0, 0},
    {0, 255, 0, 0, 0, 0},
};

s16 gMenuPlayerModelYaw[2] __attribute__((section(".sdata"))) = {0};

const u32 gGroupedSelectionMenuTablePadding
    __attribute__((section(".rodata.grouped_selection_menu_padding"))) = 0;

const u32 gMemoryCardSlotTaskTablePadding
    __attribute__((section(".rodata.memory_card_slot_task_padding"))) = 0;

StateValueHistory D_8013BE30 __attribute__((section(".bss.state_value_history")));

extern SelectionItemStats D_8012FCE4[];
extern u8 D_80130094[][5];
extern u8 D_801300A0[][5];
extern u8 D_8013B44C[];

void updateRegisteredPlayerModelTask(void);
void func_80086C34(s32 x, s32 y, s32 destination, s32 source, s32 index);

INCLUDE_ASM("asm/nonmatchings/unclassified/menu_8007CC78", func_8007CC78);

INCLUDE_ASM("asm/nonmatchings/unclassified/menu_8007CC78", func_8007D47C);

void initializeModeDependentDualResourceTask(void) {
    D_8013AE88->data18 = 1;
    if (D_8013AFCD == 1) {
        D_8013AE88->data1A = 1;
    } else {
        D_8013AE88->data1A = 3;
    }
    if (D_8013AE88->resource14 != 0xFFFF) {
        while (1) {}
    }
    D_8013AE88->resource14 = allocateTaskResource();
    if (D_8013AE88->resource16 != 0xFFFF) {
        while (1) {}
    }
    D_8013AE88->resource16 = allocateTaskResource();
    setCurrentTaskCallback(func_8007D5C8);
}

INCLUDE_ASM("asm/nonmatchings/unclassified/menu_8007CC78", func_8007D5C8);

static inline PaletteRotationSlot *getPaletteRotationSlot(u16 resourceIndex, s32 index) {
    PaletteRotationSlot *slots = (PaletteRotationSlot *)&D_800D9504[resourceIndex];
    return &slots[index / 10];
}

void createPaletteRotationTask(s32 x, s32 y, s32 first, s32 last, s32 steps, s32 interval, s32 index) {
    PaletteRotationResource *resource;
    PaletteRotationSlot *slot;
    func_80014874(updatePaletteRotationTask);
    D_8013AE8C->data18 = 0;
    D_8013AE8C->data1E = index;
    D_8013AE8C->resource14 = allocateTaskResource();
    resource = (PaletteRotationResource *)&D_800D9504[D_8013AE8C->resource14];
    resource->rect.w = 16;
    resource->rect.x = x;
    resource->rect.y = y;
    resource->rect.h = 1;
    StoreImage(&resource->rect, (u32 *)resource->original);
    resource->interval = interval;
    resource->timer = 0;
    resource->first = first;
    resource->last = last;
    resource->steps = steps;
    if (index % 10 == 0) {
        if (D_8013AE88->resource14 == 0xFFFF) { while (1) {} }
        slot = getPaletteRotationSlot(D_8013AE88->resource14, index);
    } else {
        if (D_8013AE88->resource16 == 0xFFFF) { while (1) {} }
        slot = getPaletteRotationSlot(D_8013AE88->resource16, index);
    }
    slot->state = 1;
    slot->unk06 = 0;
    slot->task = D_8013AE8C;
}

static inline PaletteRotationSlot *getPaletteEntrySlot(u16 resourceIndex, u16 index) {
    PaletteRotationSlot *slots = (PaletteRotationSlot *)&D_800D9504[resourceIndex];
    return &slots[(u16)(index / 10)];
}

void updatePaletteRotationTask(void) {
    PaletteRotationResource *resource;
    PaletteRotationSlot *slot;
    s32 i, j, step;
    u16 color;
    if (D_8013AE88->resource14 == 0xFFFF) { while (1) {} }
    resource = (PaletteRotationResource *)&D_800D9504[D_8013AE88->resource14];
    if ((u16)(D_8013AE88->data1E % 10) == 0) {
        if (D_8013AE98->resource14 == 0xFFFF) { while (1) {} }
        slot = getPaletteEntrySlot(D_8013AE98->resource14, D_8013AE88->data1E);
    } else {
        if (D_8013AE98->resource16 == 0xFFFF) { while (1) {} }
        slot = getPaletteEntrySlot(D_8013AE98->resource16, D_8013AE88->data1E);
    }
    switch (slot->state) {
    case 1:
        if (++resource->timer < 2) return;
        for (i = 0; i < 16; i++) resource->rotated[i] = resource->original[i];
        resource->timer = 0;
        slot->state = 0;
        break;
    case 99:
        LoadImage(&resource->rect, (u32 *)resource->original);
        slot->task = 0;
        slot->unk06 = 0;
        func_800149A8();
        return;
    }
    if (++resource->timer > resource->interval) {
        resource->timer = 0;
        for (step = 0; step < resource->steps; step++) {
            color = resource->rotated[resource->last];
            for (j = resource->last; j > resource->first; j--) resource->rotated[j] = resource->rotated[j - 1];
            resource->rotated[resource->first] = color;
        }
        LoadImage(&resource->rect, (u32 *)resource->rotated);
    }
}

INCLUDE_ASM("asm/nonmatchings/unclassified/menu_8007CC78", func_8007E298);

INCLUDE_ASM("asm/nonmatchings/unclassified/menu_8007CC78", func_8007E8CC);

void drawSelectionItemDetails(void) {
    Movement2D *position;
    HalfwordPair *selection;
    s16 index;
    ByteMappingDescriptor *group;
    SelectionItemStats *stats;

    if (D_8013AE88->resource14 == 0xFFFF) {
        for (;;) {}
    }
    position = (Movement2D *)&D_800D9504[D_8013AE88->resource14];
    if (D_8013AE98->resource14 == 0xFFFF) {
        for (;;) {}
    }
    selection = (HalfwordPair *)&D_800D9504[D_8013AE98->resource14];
    if (D_8013AE88->data1A == 0) {
        return;
    }
    func_8006C998(position->x / 4096 + 8, position->y / 4096 - 22,
                 0xB3, 0, -1, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
    group = &D_8012FD10[D_8013AE98->data1A];
    index = func_8007373C(group->mapping, selection->unk_02);
    if ((u16)index >= 10) {
        return;
    }
    stats = &D_8012FCE4[index];
    if (stats->price && stats->rating) {
        func_8006C998(position->x / 4096 + 84, position->y / 4096 + 128,
                     0xBC, 10, -1, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
        drawDecimalNumber8x8(position->x / 4096 + 88, position->y / 4096 + 136,
                             4, 0, stats->price, 0, -1, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
        func_8006C998(position->x / 4096 + 122, position->y / 4096 + 138,
                     0xBF, 10, -1, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
        func_8006C998(position->x / 4096 + 84, position->y / 4096 + 148,
                     0xBE, 10, -1, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
        drawHalfStepSpriteBar(position->x / 4096 + 88, position->y / 4096 + 156,
                              stats->rating, 5, -1, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
    }
    if (D_8013AE88->data1A == 1) {
        func_8007240C(position->x / 4096 + 80, position->y / 4096 + 124, 56, 44,
                      D_8013B3A4, D_8013B3A5, D_8013B3A6, 2,
                      &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
    } else {
        func_8007240C(position->x / 4096 + 80, position->y / 4096 + 124, 56, 44,
                      128, 128, 128, 2, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
    }
}

INCLUDE_ASM("asm/nonmatchings/unclassified/menu_8007CC78", func_8007EE68);

void clearHalfwordPairTable(void) {
    s16 i;

    for (i = 0; i < 16; i++) {
        D_8012F13C[i].unk_00 = 0;
        D_8012F13C[i].unk_02 = 0;
    }
}

INCLUDE_ASM("asm/nonmatchings/unclassified/menu_8007CC78", func_8007F46C);

void initializeCountedPlayerGroupedSelectionBoundsTask(void) {
    RECT *bounds;
    D_8013AE98->data1C++;
    if (D_8013AFCD == 2) {
        setCurrentTaskCallback(initializeDualResourceTaskAndResetByteState);
    } else {
        D_8013AE88->data18 = 0;
        D_8013AE88->data1A = getPlayerRecordFlagMode(0);
        if (D_8013AE88->resource14 != 0xFFFF) { for (;;) {} }
        D_8013AE88->resource14 = allocateTaskResource();
        if (D_8013AE88->resource16 != 0xFFFF) { for (;;) {} }
        D_8013AE88->resource16 = allocateTaskResource();
        adjustGroupedIndexForMode(D_8013AE88->data1A, &gRacePlayers[0].unk00A[1]);
        D_801300AC[0].selection = getGroupedIndexBase(gRacePlayers[0].unk00A[1]);
        if (D_801300AC[0].selection == 9) {
            D_801300AC[0].selection += getSaveRecordFlagGroupIndex(0, 0);
        }
        D_801300B8[0].x = 0;
        D_801300B8[0].y = 0;
        D_801300B8[0].w = -1;
        D_801300B8[0].h = -1;
        D_801300B8[1].x = 0;
        bounds = &D_801300B8[1];
        bounds->y = 0;
        bounds->w = -1;
        bounds->h = -1;
        D_801300AC[0].previousSelection = D_801300AC[0].selection;
        setCurrentTaskCallback(func_8007F630);
    }
}

INCLUDE_ASM("asm/nonmatchings/unclassified/menu_8007CC78", func_8007F630);

void initializeDualResourceTaskAndClearState(void) {
    D_8013AE88->data18 = 0;
    if (D_8013AE88->resource14 != 0xFFFF) {
        while (1) {}
    }
    D_8013AE88->resource14 = allocateTaskResource();
    if (D_8013AE88->resource16 != 0xFFFF) {
        while (1) {}
    }
    D_8013AE88->resource16 = allocateTaskResource();
    setCurrentTaskCallback(func_800801A0);
}

INCLUDE_ASM("asm/nonmatchings/unclassified/menu_8007CC78", func_800801A0);

s32 getPlayerRecordFlagMode(s16 index) {
    s32 result;
    SaveRecord *record;
    s16 playerIndex = index;
    if (index >= 10) {
        record = (SaveRecord *)&D_8013101C;
        playerIndex = index - 10;
    } else {
        record = &((SaveRecord *)&D_8013080C)[index];
    }
    if (gRacePlayers[playerIndex].unk009 == 8) {
        result = 2;
    } else if (gRacePlayers[playerIndex].unk009 == 9) {
        result = 5;
    } else {
        result = (record->flags_3AC & 0x3800) != 0;
    }
    return result;
}

void buildRecordFlagMenuEntries(s16 index, s16 mode, s16 group, u16 arg4, u16 arg5,
                                RecordFlagMenuEntries *out) {
    SaveRecord *record;
    s32 i;

    out->unk_05 = arg5;
    out->unk_04 = arg4;
    if (index >= 10) {
        record = (SaveRecord *)&D_8013101C;
        index -= 10;
    } else {
        record = &((SaveRecord *)&D_8013080C)[index];
    }

    switch (mode) {
    case 2:
    case 5:
        out->entries = D_80130094[index];
        out->entries[0] = 8;
        out->count = 1;
        out->textIndices = D_8013B44C;
        out->unk_10 = 0;
        return;
    case 1:
        if (group != 3) {
            goto standardGroups;
        }
        out->entries = D_80130094[index];
        out->count = 0;
        out->textIndices = D_801300A0[index];
        out->unk_10 = 0;
        for (i = 0; i < 3; i++) {
            if (record->flags_3AC & (0x800 << i)) {
                out->entries[out->count] = i + 11;
                out->textIndices[out->count] = i + 5;
                out->count++;
            }
        }
        return;
    case 0:
standardGroups:
        if (group < 3) {
            out->entries = D_80130094[index];
            out->count = 0;
            out->textIndices = D_801300A0[index];
            out->unk_10 = 0;
            if (group == 0) {
                for (i = 0; i < 3; i++) {
                    if (record->flags_3AC & (4 << i)) {
                        out->entries[out->count] = i + 8;
                        out->textIndices[out->count] = 8;
                        out->count++;
                    }
                }
            } else if (group == 1) {
                for (i = 0; i < 3; i++) {
                    if (record->flags_3AC & (0x20 << i)) {
                        out->entries[out->count] = i + 8;
                        out->textIndices[out->count] = 8;
                        out->count++;
                    }
                }
            } else if (group == 2) {
                for (i = 0; i < 3; i++) {
                    if (record->flags_3AC & (0x100 << i)) {
                        out->entries[out->count] = i + 8;
                        out->textIndices[out->count] = 8;
                        out->count++;
                    }
                }
            }
            return;
        }
        /* fall through */
    default:
        out->entries = NULL;
        out->textIndices = NULL;
        out->unk_10 = 0;
        return;
    }
}

s32 getSaveRecordFlagGroupIndex(s16 recordIndex, s16 groupIndex) {
    SaveRecord *record;
    s16 i;
    if (recordIndex >= 10) {
        record = (SaveRecord *)&D_8013101C;
    } else {
        record = &((SaveRecord *)&D_8013080C)[recordIndex];
    }
    for (i = 0; i < 3; i++) {
        if (record->flags_3AC & (0x800 << (groupIndex * 3 + i))) {
            return i;
        }
    }
    return 0;
}

s16 getTaskResourceGroupedIndex(GameTask *task, s16 index) {
    s16 result;
    s32 base = 0;
    GroupedIndexResource *resource = (GroupedIndexResource *)&D_800D9504[task->resource14];
    switch (resource->values[index]) {
    case 1: result = base; break;
    case 2: result = base + 1; break;
    case 3: result = base + 2; break;
    case 4: result = base + 9; break;
    case 5:
    case 14: result = 12; break;
    case 6:
    case 15: result = 13; break;
    case 7:
    case 16: result = 14; break;
    default: result = -1; break;
    }
    return result;
}

s16 getTaskResourceAdjustedGroupedIndex(s16 value, GameTask *task, s16 index) {
    s16 result;
    s32 base;
    GroupedIndexResource *resource = (GroupedIndexResource *)&D_800D9504[task->resource14];
    switch (resource->values[index]) {
    case 8: base = 0; break;
    case 9: base = 3; break;
    case 10: base = 6; break;
    case 11: base = 0; break;
    case 12: base = 1; break;
    case 13: base = 2; break;
    default: base = 0; break;
    }
    switch (value) {
    case 1: result = base; break;
    case 2: result = base + 1; break;
    case 3: result = base + 2; break;
    case 4: result = base + 9; break;
    case 5:
    case 14: result = 12; break;
    case 6:
    case 15: result = 13; break;
    case 7:
    case 16: result = 14; break;
    }
    return result;
}

void adjustGroupedIndexForMode(s16 mode, u8 *index) {
    s32 value = *index;
    switch (mode) {
    case 2:
    case 5:
        if (value < 12) {
            value = 12;
        }
        break;
    case 0:
    case 1:
        if (value >= 12) {
            value = 0;
        }
        break;
    }
    *index = value;
}

s16 getGroupedIndexBase(s16 index) {
    s16 result;
    if (index < 9) {
        result = index % 3;
    } else if ((u16)(index - 9) < 3) {
        result = 9;
    } else if (index >= 12) {
        result = index;
    }
    return result;
}

s16 getGroupedIndexOffset(s16 index) {
    s16 result;
    if (index < 9) {
        result = index / 3;
    } else if ((u16)(index - 9) < 3) {
        result = index - 9;
    } else if (index >= 12) {
        result = index - 12;
    }
    return result;
}

s16 getGroupedIndexOffsetPlus216(s16 index) {
    s16 result;
    if (index < 9) {
        result = index / 3 + 216;
    } else if ((u16)(index - 9) < 3) {
        result = index + 210;
    } else if (index >= 12) {
        result = index + 204;
    }
    return result;
}

s16 findIndexedHalfwordRecord(IndexedHalfwordRecord *records, u8 *indices, s16 count, s16 value) {
    s16 result = 0;
    s16 i;
    for (i = 0; i < count; i++) {
        if (records[indices[i]].value == value) {
            result = i;
            break;
        }
    }
    return result;
}

static inline RegisteredPanelSlot *getPanelSlot(u32 resource, s32 index) {
    RegisteredPanelSlot *slots = (RegisteredPanelSlot *)&D_800D9504[resource];
    return &slots[index / 10];
}

void createRegisteredPanelTask(s32 mode, s32 value, s16 x, s16 y, s16 depth, s32 index) {
    Movement2D *movement;
    RegisteredPanelSlot *slot;
    func_80014874(func_800815D0);
    D_8013AE8C->data18 = mode;
    D_8013AE8C->data1A = value;
    D_8013AE8C->data1C = 0;
    D_8013AE8C->data1E = index;
    D_8013AE8C->resource14 = allocateTaskResource();
    movement = (Movement2D *)&D_800D9504[D_8013AE8C->resource14];
    initMovement2D(movement, x << 12, y << 12);
    movement->unk08 = depth;
    D_8013AE8C->resource16 = allocateTaskResource();
    if (index % 10 == 0) {
        if (D_8013AE88->resource14 == 0xFFFF) { while (1) {} }
        slot = getPanelSlot(D_8013AE88->resource14, index);
    } else {
        if (D_8013AE88->resource16 == 0xFFFF) { while (1) {} }
        slot = getPanelSlot(D_8013AE88->resource16, index);
    }
    slot->command = 0;
    slot->flags = 1;
    slot->task = D_8013AE8C;
}

INCLUDE_ASM("asm/nonmatchings/unclassified/menu_8007CC78", func_800815D0);

INCLUDE_ASM("asm/nonmatchings/unclassified/menu_8007CC78", func_800816D8);

INCLUDE_ASM("asm/nonmatchings/unclassified/menu_8007CC78", func_80081AC8);

INCLUDE_ASM("asm/nonmatchings/unclassified/menu_8007CC78", func_80081BAC);

static inline RegisteredTaskEntry *getRegisteredTripletEntry(u16 resource, s32 index) {
    RegisteredTaskEntry *entries = (RegisteredTaskEntry *)&D_800D9504[resource];
    return &entries[index / 10];
}

void createRegisteredTripletTask(s32 x, s32 y, s32 offsetX, s32 offsetY, ByteTriplet *records, u8 *indices, s32 depth, s32 index) {
    RegisteredTripletData *data;
    RegisteredTaskEntry *entry;
    func_80014874(func_80081DF0);
    D_8013AE8C->data1A = 0;
    D_8013AE8C->data1C = depth;
    D_8013AE8C->data1E = index;
    D_8013AE8C->resource14 = allocateTaskResource();
    initMovement2D((Movement2D *)&D_800D9504[D_8013AE8C->resource14], (s16)x << 12, (s16)y << 12);
    D_8013AE8C->resource16 = allocateTaskResource();
    data = (RegisteredTripletData *)&D_800D9504[D_8013AE8C->resource16];
    data->records = records;
    data->indices = indices;
    data->offsetX = offsetX;
    data->offsetY = offsetY;
    if (index % 10 == 0) {
        if (D_8013AE88->resource14 == 0xFFFF) { while (1) {} }
        entry = getRegisteredTripletEntry(D_8013AE88->resource14, index);
    } else {
        if (D_8013AE88->resource16 == 0xFFFF) { while (1) {} }
        entry = getRegisteredTripletEntry(D_8013AE88->resource16, index);
    }
    entry->command = 0;
    entry->flags = 0;
    entry->task = D_8013AE8C;
}

INCLUDE_ASM("asm/nonmatchings/unclassified/menu_8007CC78", func_80081DF0);

void createRegisteredPlayerModelTask(s16 slot, s16 selection, s16 modelIndex,
                                     s16 x, s16 y, s16 z, s32 index) {
    MenuModelResource *resource;
    u32 *offsets;
    u32 *textureOffsets;
    u32 *entry;
    u8 *archive;
    u32 (*buffers)[512];
    u32 *dest;
    u32 *source;
    u32 *model;
    u32 i;
    RegisteredTaskEntry *registration;

    func_80014874(updateRegisteredPlayerModelTask);
    D_8013AE8C->data18 = selection;
    D_8013AE8C->data1A = modelIndex;
    D_8013AE8C->data1C = slot;
    D_8013AE8C->data1E = index;
    D_8013AE8C->resource14 = allocateTaskResource();
    offsets = DAT_STARTUP_MODELS->offsets;
    archive = (u8 *)DAT_STARTUP_MODELS;
    buffers = DAT_PLAYER_MODEL_BLOCKS;
    i = 0;
    resource = (MenuModelResource *)&D_800D9504[D_8013AE8C->resource14];
    entry = &offsets[D_80123640[D_8013AE8C->data1A]];
    model = buffers[D_8013AE8C->data1C];
    source = (u32 *)(*entry + archive);
    dest = model;
    for (; i < 512; i++) {
        *dest++ = *source++;
    }
    entry = (u32 *)((u32)model | 4);
    GsMapModelingData(entry);
    entry += 2;
    if (D_8013AE8C->data1C != 0) {
        offsetTmdObjectTextures(0, (TmdObject *)entry, &gPlayerRootTextureOffsets[D_8013AE8C->data1C]);
    }
    GsLinkObject4(entry, &resource->object, 0);
    GsInitCoordinate2(0, &resource->world);
    GsInitCoordinate2(&resource->world, &resource->local);
    resource->object.coord2 = &resource->local;
    resource->object.attribute = 0;
    textureOffsets = DAT_STARTUP_TEXTURES->offsets;
    entry = &textureOffsets[gPlayerResourceTextureSelections.entries[D_8013AE8C->data18][D_8013AE8C->data1A]];
    uploadTimImageWithOffset((u32 *)((u8 *)DAT_STARTUP_TEXTURES + *entry),
                             gPlayerResourceTexturePositions[D_8013AE8C->data1C]);
    resource->world.coord.t[0] = x;
    resource->world.coord.t[1] = y;
    resource->world.coord.t[2] = z;
    resource->worldRotation.vx = 0;
    resource->worldRotation.vy = 0;
    resource->worldRotation.vz = 0;
    gMenuPlayerModelYaw[(s16)selection] = resource->worldRotation.vy;
    resource->local.coord.t[0] = 0;
    resource->local.coord.t[1] = 0;
    resource->local.coord.t[2] = 0;
    resource->localRotation.vx = 0x400;
    resource->localRotation.vy = -0x249;
    resource->localRotation.vz = 0;
    resource->unkC2 = 0;
    D_8013AE8C->resource16 = allocateTaskResource();
    initMovement2D((Movement2D *)&D_800D9504[D_8013AE8C->resource16], x << 12, y << 12);
    if (index % 10 == 0) {
        if (D_8013AE88->resource14 == 0xFFFF) { while (1) {} }
        registration = getRegisteredTripletEntry(D_8013AE88->resource14, index);
    } else {
        if (D_8013AE88->resource16 == 0xFFFF) { while (1) {} }
        registration = getRegisteredTripletEntry(D_8013AE88->resource16, index);
    }
    registration->command = 0;
    registration->flags = 1;
    registration->task = D_8013AE8C;
}

void reloadTaskPlayerModel(GameTask *task, s16 slot, s16 selection, s16 modelIndex, s16 x, s16 y, s16 z) {
    MenuModelResource saved;
    MenuModelResource *resource;
    u32 *offsets;
    u32 *textureOffsets;
    u32 *entry;
    u8 *archive;
    u32 (*buffers)[512];
    u32 *dest;
    u32 *source;
    u32 *model;
    u32 i;
    task->data18 = selection;
    task->data1A = modelIndex;
    task->data1C = slot;
    resource = (MenuModelResource *)&D_800D9504[task->resource14];
    saved = *resource;
    offsets = DAT_STARTUP_MODELS->offsets;
    archive = (u8 *)DAT_STARTUP_MODELS;
    buffers = DAT_PLAYER_MODEL_BLOCKS;
    i = 0;
    entry = &offsets[D_80123640[task->data1A]];
    source = (u32 *)(*entry + archive);
    model = buffers[task->data1C];
    dest = model;
    for (; i < 512; i++) { *dest++ = *source++; }
    entry = (u32 *)((u32)model | 4);
    GsMapModelingData(entry);
    entry += 2;
    if (task->data1C != 0) {
        offsetTmdObjectTextures(0, (TmdObject *)entry, &gPlayerRootTextureOffsets[task->data1C]);
    }
    GsLinkObject4(entry, &resource->object, 0);
    GsInitCoordinate2(0, &resource->world);
    GsInitCoordinate2(&resource->world, &resource->local);
    resource->object.coord2 = &resource->local;
    resource->object.attribute = 0;
    textureOffsets = DAT_STARTUP_TEXTURES->offsets;
    entry = &textureOffsets[gPlayerResourceTextureSelections.entries[task->data18][task->data1A]];
    uploadTimImageWithOffset((u32 *)((u8 *)DAT_STARTUP_TEXTURES + *entry), gPlayerResourceTexturePositions[task->data1C]);
    resource->world.coord.t[0] = x;
    resource->world.coord.t[1] = y;
    resource->world.coord.t[2] = z;
    resource->worldRotation.vx = saved.worldRotation.vx;
    resource->worldRotation.vy = saved.worldRotation.vy;
    resource->worldRotation.vz = saved.worldRotation.vz;
    resource->local.coord.t[0] = saved.local.coord.t[0];
    resource->local.coord.t[1] = saved.local.coord.t[1];
    resource->local.coord.t[2] = saved.local.coord.t[2];
    resource->localRotation.vx = saved.localRotation.vx;
    resource->localRotation.vy = saved.localRotation.vy;
    resource->localRotation.vz = saved.localRotation.vz;
    resource->unkC2 = saved.unkC2;
    initMovement2D((Movement2D *)&D_800D9504[task->resource16], x << 12, y << 12);
}

static inline RegisteredTaskEntry *getRegisteredMenuTaskEntry(u16 resource, u32 index) {
    RegisteredTaskEntry *entries = (RegisteredTaskEntry *)&D_800D9504[resource];
    return &entries[(u16)(index / 10)];
}

void updateRegisteredPlayerModelTask(void) {
    MenuModelResource *model;
    Movement2D *movement;
    RegisteredTaskEntry *entry;
    u8 padding[16];

    if (D_8013AE88->resource14 == 0xFFFF) { while (1) {} }
    model = (MenuModelResource *)&D_800D9504[D_8013AE88->resource14];
    if (D_8013AE88->resource16 == 0xFFFF) { while (1) {} }
    movement = (Movement2D *)&D_800D9504[D_8013AE88->resource16];

    if ((u16)(D_8013AE88->data1E % 10) == 0) {
        if (D_8013AE98->resource14 == 0xFFFF) { while (1) {} }
        entry = getRegisteredMenuTaskEntry(D_8013AE98->resource14, D_8013AE88->data1E);
    } else {
        if (D_8013AE98->resource16 == 0xFFFF) { while (1) {} }
        entry = getRegisteredMenuTaskEntry(D_8013AE98->resource16, D_8013AE88->data1E);
    }

    {
        s16 command = entry->command;
        if (command == 2) goto keepRotating;
        if (command < 3) {
            if (command == 0) goto clearFlags;
            if (command == 1) goto startRotating;
            goto remove;
        }
        if (command == 3) goto resetRotation;
        goto remove;
    }

startRotating:
    model->unkC2 = -32;
    entry->command = 0;
    entry->flags = 0;
    if (D_8013AE88->data18 == 0) {
        model->worldRotation.vy = gMenuPlayerModelYaw[1];
    } else {
        model->worldRotation.vy = gMenuPlayerModelYaw[0] - model->unkC2;
    }
    goto update;

keepRotating:
    model->unkC2 = -32;
    goto clearCommand;

resetRotation:
    model->worldRotation.vy = 0;
    model->unkC2 = 0;
clearCommand:
    entry->command = 0;
clearFlags:
    entry->flags = 0;
    goto update;

remove:
    entry->task = 0;
    entry->flags = 0;
    func_800149A8();
    return;

update:
    entry->flags |= updateMovement2D(movement);
    model->world.coord.t[0] = movement->x / 4096;
    model->world.coord.t[1] = movement->y / 4096;
    model->worldRotation.vy += model->unkC2;
    gMenuPlayerModelYaw[D_8013AE88->data18] = model->worldRotation.vy;
    func_8001E3E4(&model->worldRotation, &model->world);
    func_8001E3E4(&model->localRotation, &model->local);
    if (model->world.coord.t[2] >= 512) {
        sortLitObjectWithDrawArea(&model->object, D_8013AE88->data18 + 2, &D_801300B8[D_8013AE88->data18]);
    } else {
        u16 index = D_8013AE88->data1C;
        sortLitObjectWithDrawArea(&model->object, index & 1, &D_801300B8[index]);
    }
}

void sortLitObjectWithDrawArea(GsDOBJ2 *object, u16 index, RECT *bounds) {
    MATRIX matrix;
    GsGetLw(object->coord2, &matrix);
    GsSetLightMatrix(&matrix);
    GsGetLs(object->coord2, &matrix);
    GsSetLsMatrix(&matrix);
    {
        u32 depthIndex = index;
        GsSortObject4(object, &D_800EBFF0[D_8013AF78], (depthIndex & 1) + 1,
                      (u32 *)getScratchAddr(0));
        if (bounds->w >= 0 && bounds->h >= 0) {
            addDrawAreaPacket(bounds->x, bounds->y, bounds->w, bounds->h,
                              &D_800EBFF0[D_8013AF78], litObjectDrawAreaDepths[depthIndex]);
            addDrawAreaPacket(0, 0, 320, 240, &D_800EBFF0[D_8013AF78], 14);
        }
    }
}

ByteTriplet *lookupByteTriplet(ByteTriplet *records, u8 *indices, s16 index) {
    return &records[indices[index]];
}

void transitionRegisteredPlayerModels(void) {
    MenuPlayerModelPosition *position;
    RegisteredTaskEntry *children;
    s16 player;
    RegisteredTaskEntry *entry;
    MenuSelectionState *selection;
    u16 playerIndex;
    s32 command;

    if (D_8013AE88->resource14 == 0xFFFF) { while (1) {} }
    position = (MenuPlayerModelPosition *)&D_800D9504[D_8013AE88->resource14];
    if (D_8013AE88->resource16 == 0xFFFF) { while (1) {} }
    children = (RegisteredTaskEntry *)&D_800D9504[D_8013AE88->resource16];
    if ((u16)(D_8013AE88->data1E % 10) == 0) {
        if (D_8013AE98->resource14 == 0xFFFF) { while (1) {} }
        entry = getRegisteredMenuTaskEntry(D_8013AE98->resource14, D_8013AE88->data1E);
    } else {
        if (D_8013AE98->resource16 == 0xFFFF) { while (1) {} }
        entry = getRegisteredMenuTaskEntry(D_8013AE98->resource16, D_8013AE88->data1E);
    }
    if (D_8013AE98->resource14 == 0xFFFF) { while (1) {} }
    selection = &D_801300AC[(s16)D_8013AE88->data18];
    playerIndex = D_8013AE88->data18;
    command = entry->command;
    if (command == 1) {
        s16 incomingSlot;
        s32 outgoingSlot;
        Movement2D *movement;
        if (D_8013AE88->data1C == 0) {
            outgoingSlot = 0;
            incomingSlot = 1;
            D_8013AE88->data1C = command;
        } else {
            outgoingSlot = 1;
            incomingSlot = 0;
            D_8013AE88->data1C = 0;
        }
        movement = (Movement2D *)&D_800D9504[children[incomingSlot].task->resource16];
        player = playerIndex;
        reloadTaskPlayerModel(children[incomingSlot].task, (s16)(incomingSlot + player * 2), player,
                              selection->selection, position->x / 4096, position->y / 4096 - 180, position->z);
        configureMovement2D(movement, position->x, position->y, 0x4B000, 0x4B000, 0);

        movement = (Movement2D *)&D_800D9504[children[outgoingSlot].task->resource16];
        reloadTaskPlayerModel(children[outgoingSlot].task, (s16)(outgoingSlot + player * 2), player,
                              selection->previousSelection, position->x / 4096, movement->y / 4096, position->z);
        configureMovement2D(movement, position->x, position->y + 0xB4000, 0x4B000, 0x4B000, 0);
        entry->command = 0;
        entry->flags = 0;
        selection->previousSelection = selection->selection;
        setCurrentTaskCallback(func_800816D8);
    } else {
        setCurrentTaskCallback(func_800816D8);
        entry->command = 0;
        entry->flags = 0;
    }
}

void transitionRegisteredPlayerModelsUp(void) {
    MenuPlayerModelPosition *position;
    RegisteredTaskEntry *children;
    s16 player;
    RegisteredTaskEntry *entry;
    MenuSelectionState *selection;
    u16 playerIndex;
    s32 command;

    if (D_8013AE88->resource14 == 0xFFFF) { while (1) {} }
    position = (MenuPlayerModelPosition *)&D_800D9504[D_8013AE88->resource14];
    if (D_8013AE88->resource16 == 0xFFFF) { while (1) {} }
    children = (RegisteredTaskEntry *)&D_800D9504[D_8013AE88->resource16];
    if ((u16)(D_8013AE88->data1E % 10) == 0) {
        if (D_8013AE98->resource14 == 0xFFFF) { while (1) {} }
        entry = getRegisteredMenuTaskEntry(D_8013AE98->resource14, D_8013AE88->data1E);
    } else {
        if (D_8013AE98->resource16 == 0xFFFF) { while (1) {} }
        entry = getRegisteredMenuTaskEntry(D_8013AE98->resource16, D_8013AE88->data1E);
    }
    if (D_8013AE98->resource14 == 0xFFFF) { while (1) {} }
    selection = &D_801300AC[(s16)D_8013AE88->data18];
    playerIndex = D_8013AE88->data18;
    command = entry->command;
    if (command == 1) {
        s16 incomingSlot;
        s32 outgoingSlot;
        Movement2D *movement;
        if (D_8013AE88->data1C == 0) {
            outgoingSlot = 0;
            incomingSlot = 1;
            D_8013AE88->data1C = command;
        } else {
            outgoingSlot = 1;
            incomingSlot = 0;
            D_8013AE88->data1C = 0;
        }
        movement = (Movement2D *)&D_800D9504[children[incomingSlot].task->resource16];
        player = playerIndex;
        reloadTaskPlayerModel(children[incomingSlot].task, (s16)(incomingSlot + player * 2), player,
                              selection->selection, position->x / 4096, position->y / 4096 + 180, position->z);
        configureMovement2D(movement, position->x, position->y, 0x4B000, 0x4B000, 0);

        movement = (Movement2D *)&D_800D9504[children[outgoingSlot].task->resource16];
        reloadTaskPlayerModel(children[outgoingSlot].task, (s16)(outgoingSlot + player * 2), player,
                              selection->previousSelection, position->x / 4096, movement->y / 4096, position->z);
        configureMovement2D(movement, position->x, position->y - 0xB4000, 0x4B000, 0x4B000, 0);
        entry->command = 0;
        entry->flags = 0;
        selection->previousSelection = selection->selection;
        setCurrentTaskCallback(func_800816D8);
    } else {
        setCurrentTaskCallback(func_800816D8);
        entry->command = 0;
        entry->flags = 0;
    }
}

void updateRegisteredChildBlinkTransition(void) {
    Movement2D *position;
    RegisteredTaskEntry *children;
    RegisteredTaskEntry *entry;
    SelectionTransitionEntry *selection;
    if (D_8013AE88->resource14 == 0xFFFF) { while (1) {} }
    position = (Movement2D *)&D_800D9504[D_8013AE88->resource14];
    if (D_8013AE88->resource16 == 0xFFFF) { while (1) {} }
    children = (RegisteredTaskEntry *)&D_800D9504[D_8013AE88->resource16];
    if ((u16)(D_8013AE88->data1E % 10) == 0) {
        if (D_8013AE98->resource14 == 0xFFFF) { while (1) {} }
        entry = getRegisteredMenuTaskEntry(D_8013AE98->resource14, D_8013AE88->data1E);
    } else {
        if (D_8013AE98->resource16 == 0xFFFF) { while (1) {} }
        entry = getRegisteredMenuTaskEntry(D_8013AE98->resource16, D_8013AE88->data1E);
    }
    selection = &((SelectionTransitionEntry *)&D_801300AC)[(s16)D_8013AE88->data18];
    if (entry->command == 1) {
        if (selection->timer == 0) {
            initMovement2D((Movement2D *)&D_800D9504[children[D_8013AE88->data1C].task->resource16], position->x, position->y);
            entry->command = 0;
            entry->flags = 0;
        } else if (selection->timer > 0) {
            if (selection->timer % 2 == 0) {
                initMovement2D((Movement2D *)&D_800D9504[children[D_8013AE88->data1C].task->resource16], position->x, position->y);
            } else {
                initMovement2D((Movement2D *)&D_800D9504[children[D_8013AE88->data1C].task->resource16], position->x + 0x140000, position->y);
            }
            selection->timer--;
            entry->flags = 1;
        }
    } else {
        setCurrentTaskCallback(func_800816D8);
        entry->command = 0;
        entry->flags = 0;
        selection->previousSelection = selection->selection;
    }
}

INCLUDE_ASM("asm/nonmatchings/unclassified/menu_8007CC78", func_80083C3C);

void drawTaskOffsetSpriteB8(void) {
    Movement2D *resource;
    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (Movement2D *)&D_800D9504[D_8013AE88->resource14];
    if (D_8013AE88->data1A != 0) {
        func_8006C998(resource->x / 4096 + 8, resource->y / 4096 - 22,
                      0xB8, 0, -1, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
    }
}

void initializeDualResourceTaskAndResetByteState(void) {
    D_8013AE88->data18 = 1;
    D_8013B6B8[0] = 0;
    D_8013B6B9[0] = 0;
    if (D_8013AE88->resource14 != 0xFFFF) {
        while (1) {}
    }
    D_8013AE88->resource14 = allocateTaskResource();
    if (D_8013AE88->resource16 != 0xFFFF) {
        while (1) {}
    }
    D_8013AE88->resource16 = allocateTaskResource();
    setCurrentTaskCallback(updatePlayerGroupedSelectionMenu);
    D_8013AE88->unk10 |= 2;
}

void updatePlayerGroupedSelectionMenu(void) {
    RegisteredPanelSlot *slots;

    if (D_8013AE88->resource14 == 0xFFFF) { while (1) {} }
    if (D_8013AE88->resource16 == 0xFFFF) { while (1) {} }
    slots = (RegisteredPanelSlot *)&D_800D9504[D_8013AE88->resource16];

    switch (D_8013AE88->data18) {
    case 1:
        setMusicSequenceEnabled(1);
        createPlayerGroupedSelectionTask(0, 1);
        createPlayerGroupedSelectionTask(1, 11);
        func_80086C34(816, 240, 15, 0, 21);
        func_80086C34(816, 241, 15, 0, 31);
        D_8013AE88->data18 = 3;
        break;
    case 3:
        if (D_8013B6B8[0] == 4 || D_8013B6B9[0] == 4) {
            D_8013AE88->data18 = 40;
        } else if (D_8013B6B8[0] == 6 || D_8013B6B9[0] == 6) {
            if (D_8013AFE2 == 0 && (s16)getGameOptionFlag0() != 0) {
                D_8013AE98->data18 = 29;
            } else {
                D_8013AE98->data18 = 23;
                beginMode2SequenceAndFadeMusic();
            }
            D_8013AE88->data18 = 9;
        }
        break;
    case 40:
        if (D_8013B6B8[0] == 4 && D_8013B6B9[0] == 4) {
            D_8013B6B8[0] = 5;
            D_8013B6B9[0] = 5;
            D_8013AE88->data18 = 41;
        }
        break;
    case 41:
        if (D_8013B6B8[0] == 6 || D_8013B6B9[0] == 6) {
            switch (D_8013AFE2) {
            case 0:
            case 2:
                D_8013AE98->data18 = 7;
                break;
            case 1:
                D_8013AE98->data18 = 20;
                break;
            }
            queueMenuResourceReads((s16)D_8013AE98->data18);
            slots[2].command = 99;
            slots[3].command = 99;
            D_8013AE88->data18 = 99;
        }
        break;
    case 9:
        if (D_8013AE98->data18 != 23 || func_800707F0(0) == 0) {
            D_8013AE88->data18 = 99;
        }
        break;
    default:
        queueMenuResourceReads((s16)D_8013AE98->data18);
        D_8013AE98->data1C--;
        func_800149A8();
        break;
    }
}

void createPlayerGroupedSelectionTask(s32 player, s32 index) {
    MenuSelectionState *selection;
    RegisteredPanelSlot *slot;
    RECT *bounds;
    func_80014874(func_80084970);
    D_8013AE8C->data18 = 1;
    D_8013AE8C->data1A = getPlayerRecordFlagMode(player + 10);
    D_8013AE8C->data1C = player;
    D_8013AE8C->data1E = index;
    D_8013AE8C->resource14 = allocateTaskResource();
    D_8013AE8C->resource16 = allocateTaskResource();
    selection = &D_801300AC[(s16)player];
    adjustGroupedIndexForMode((s16)D_8013AE8C->data1A, &((PlayerSelectionRecord *)gRacePlayers)[(s16)player].selectionBytes[3]);
    selection->selection = getGroupedIndexBase(((PlayerSelectionRecord *)gRacePlayers)[(s16)player].selectionBytes[3]);
    if (selection->selection == 9) {
        selection->selection += getSaveRecordFlagGroupIndex(player + 10, 0);
    }
    selection->previousSelection = selection->selection;
    D_801300B8[0].x = 25;
    D_801300B8[0].y = 32;
    D_801300B8[0].w = 270;
    D_801300B8[0].h = 87;
    D_801300B8[1].x = 25;
    bounds = &D_801300B8[1];
    bounds->y = 136;
    bounds->w = 270;
    bounds->h = 87;
    if (index % 10 == 0) {
        if (D_8013AE88->resource14 == 0xFFFF) { while (1) {} }
        slot = getPanelSlot(D_8013AE88->resource14, index);
    } else {
        if (D_8013AE88->resource16 == 0xFFFF) { while (1) {} }
        slot = getPanelSlot(D_8013AE88->resource16, index);
    }
    slot->command = 0;
    slot->flags = 1;
    slot->task = D_8013AE8C;
}

INCLUDE_ASM("asm/nonmatchings/unclassified/menu_8007CC78", func_80084970);

void initializeDualResourceTask(void) {
    if (D_8013AE88->resource14 != 0xFFFF) {
        while (1) {}
    }
    D_8013AE88->resource14 = allocateTaskResource();
    if (D_8013AE88->resource16 != 0xFFFF) {
        while (1) {}
    }
    D_8013AE88->resource16 = allocateTaskResource();
    setCurrentTaskCallback(func_800857C4);
}

INCLUDE_ASM("asm/nonmatchings/unclassified/menu_8007CC78", func_800857C4);

void drawClippedRegisteredTaskGridSprite(void) {
    Movement2D *position;
    ByteTriplet *color;
    ClippedGridSpriteParent *parent;
    RegisteredTaskEntry *entry;
    RegisteredTaskEntry *entries;
    const GridSpriteEntry *sprite;
    s16 row;

    if (D_8013AE88->resource14 == 0xFFFF) { for (;;) {} }
    position = (Movement2D *)&D_800D9504[D_8013AE88->resource14];
    if (D_8013AE88->resource16 == 0xFFFF) { for (;;) {} }
    color = (ByteTriplet *)&D_800D9504[D_8013AE88->resource16];
    if (D_8013AE98->resource14 == 0xFFFF) { for (;;) {} }
    parent = (ClippedGridSpriteParent *)&D_800D9504[D_8013AE98->resource14];
    if ((u16)(D_8013AE88->data1E % 10U) == 0) {
        if (D_8013AE98->resource14 == 0xFFFF) { for (;;) {} }
        entries = (RegisteredTaskEntry *)&D_800D9504[D_8013AE98->resource14];
        entry = &entries[D_8013AE88->data1E / 10U];
    } else {
        if (D_8013AE98->resource16 == 0xFFFF) { for (;;) {} }
        entries = (RegisteredTaskEntry *)&D_800D9504[D_8013AE98->resource16];
        entry = &entries[D_8013AE88->data1E / 10U];
    }
    if (parent->mapping != 0) {
        if (entry->command == 99) {
            entry->task = 0;
            func_800149A8();
        } else {
            row = (parent->mode & 1) ^ 1;
            if (parent->alternateSprites == 1) {
                sprite = func_800736D0(parent->sprites, parent->mapping,
                    (s16)(D_8013AE88->data1A + parent->count));
            } else {
                sprite = func_800736D0(parent->sprites, parent->mapping, (s16)D_8013AE88->data1A);
            }
            setSpriteColor(color->bytes[0], color->bytes[1], color->bytes[2]);
            addDrawAreaPacket(0, 0, 320, 240, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
            func_8006C998((s16)(position->x / 4096), (s16)(position->y / 4096),
                sprite->sprite, sprite->color, -1, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
            addDrawAreaPacket(25, row * 104 + 40, 270, 94,
                &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
            entry->flags = 0;
        }
    }
}

void drawTaskIndexPanel(void) {
    Movement2D *resource;
    s16 index;
    TexturedQuadListEntry *list;

    if (D_8013AE88->resource14 == 0xFFFF) { while (1) {} }
    resource = (Movement2D *)&D_800D9504[D_8013AE88->resource14];
    index = D_8013AE88->data18;
    if (D_8013AE88->data1A != 0) {
        func_8006C998(resource->x / 4096 + 8, resource->y / 4096 + 80,
                      0x133, 10, -1, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
        func_8006C998(resource->x / 4096 + 24, resource->y / 4096 + 80,
                      0x31, 2, -1, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
    }
    setSpriteColor(128, 128, 128);
    setTexturedQuadColor(128, 128, 128);
    drawDecimalNumber8x8(resource->x / 4096 + 54, resource->y / 4096 + 4,
                  1, 0, index + 1, 0, -1, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
    if (index == 0) list = taskIndexPanelQuadsPalette0;
    else list = taskIndexPanelQuadsPalette1;
    drawTexturedQuadList(resource->x / 4096, resource->y / 4096, list,
                        -1, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
}

s16 mapGroupedValueToTableIndex(s16 value) {
    s16 result;

    if (value < 9) {
        result = value / 3 + 231;
    } else if ((u16)(value - 9) < 3) {
        result = value + 225;
    } else if (value >= 12) {
        result = value + 219;
    }
    return result;
}

INCLUDE_ASM("asm/nonmatchings/unclassified/menu_8007CC78", func_80086C34);

void updatePaletteEntryTask(void) {
    PaletteEntryResource *resource;
    PaletteRotationSlot *slot;
    s32 i;
    if (D_8013AE88->resource14 == 0xFFFF) { while (1) {} }
    resource = (PaletteEntryResource *)&D_800D9504[D_8013AE88->resource14];
    if ((u16)(D_8013AE88->data1E % 10) == 0) {
        if (D_8013AE98->resource14 == 0xFFFF) { while (1) {} }
        slot = getPaletteEntrySlot(D_8013AE98->resource14, D_8013AE88->data1E);
    } else {
        if (D_8013AE98->resource16 == 0xFFFF) { while (1) {} }
        slot = getPaletteEntrySlot(D_8013AE98->resource16, D_8013AE88->data1E);
    }
    switch (slot->state) {
    case 1:
        if (++D_8013AE88->data18 >= 2) {
            for (i = 0; i < 16; i++) resource->modified[i] = resource->original[i];
            resource->modified[resource->destination] = resource->original[resource->source];
            LoadImage(&resource->rect, (u32 *)resource->modified);
            slot->state = 0;
        }
        break;
    case 99:
        LoadImage(&resource->rect, (u32 *)resource->original);
        slot->task = 0;
        slot->unk06 = 0;
        func_800149A8();
        break;
    }
}

s32 isValue16(s32 value) {
    return value == 16;
}

s16 hasStateValueChanged(s16 index) {
    return D_8013BE30.current[index] != D_8013BE30.previous[index];
}

void updateMemoryCardSlotTask(void) {
    switch (D_8013AE88->data18) {
    case 0:
        resetMemoryCardTaskState();
        if (D_8013BE30.unk_00 == 0) {
            D_8013BE30.activeIndex = isValue16(D_8013BE30.unk_00 = 16);
        } else {
            D_8013BE30.activeIndex = isValue16(D_8013BE30.unk_00 = 0);
        }
        func_800198C4(D_8013BE30.unk_00);
        D_8013AE88->data18 = 1;
        break;
    case 1:
        if (D_8013AF3E != 7) {
            D_8013BE30.unk_16[D_8013BE30.activeIndex] = 1;
        } else {
            D_8013BE30.unk_16[D_8013BE30.activeIndex] = 0;
        }
        switch (D_8013AF3E) {
        case 0:
            if (D_8013BE30.current[D_8013BE30.activeIndex] != D_8013AF3E) {
                D_8013BE30.previous[D_8013BE30.activeIndex] =
                    D_8013BE30.current[D_8013BE30.activeIndex];
                D_8013BE30.current[D_8013BE30.activeIndex] = D_8013AF3E;
            }
            if (D_8013BE30.unk_0E[D_8013BE30.activeIndex] == 2) {
                D_8013BE30.unk_0E[D_8013BE30.activeIndex] = 3;
                D_8013AE88->data18 = 0;
            }
            break;
        case 2:
        case 3:
            if (D_8013BE30.current[D_8013BE30.activeIndex] != D_8013AF3E) {
                D_8013BE30.previous[D_8013BE30.activeIndex] =
                    D_8013BE30.current[D_8013BE30.activeIndex];
                D_8013BE30.current[D_8013BE30.activeIndex] = D_8013AF3E;
            }
            break;
        case 6:
            D_8013AF3E = 5;
            /* Fall through. */
        case 5:
            if (D_8013BE30.unk_0E[D_8013BE30.activeIndex] == 3) {
                D_8013AE88->data18 = 0;
            } else if (D_8013BE30.unk_12[D_8013BE30.activeIndex] == 1) {
                D_8013BE30.unk_12[D_8013BE30.activeIndex] = 0;
                D_8013BE30.unk_0E[D_8013BE30.activeIndex] = 2;
                startMemoryCardReadTask();
                D_8013BE30.previous[D_8013BE30.activeIndex] =
                    D_8013BE30.current[D_8013BE30.activeIndex];
                D_8013BE30.current[D_8013BE30.activeIndex] = D_8013AF3E;
                return;
            } else {
                if (D_8013BE30.current[D_8013BE30.activeIndex] != D_8013AF3E) {
                    D_8013BE30.previous[D_8013BE30.activeIndex] =
                        D_8013BE30.current[D_8013BE30.activeIndex];
                    D_8013BE30.current[D_8013BE30.activeIndex] = D_8013AF3E;
                }
                D_8013BE30.unk_0E[D_8013BE30.activeIndex] = 1;
                D_8013AE88->data18 = 0;
            }
            break;
        case 7:
        case 8:
        case 9:
        case 10:
        case 11:
        case 12:
            if (D_8013BE30.current[D_8013BE30.activeIndex] != D_8013AF3E) {
                D_8013BE30.previous[D_8013BE30.activeIndex] =
                    D_8013BE30.current[D_8013BE30.activeIndex];
                D_8013BE30.current[D_8013BE30.activeIndex] = D_8013AF3E;
            }
            D_8013BE30.unk_0E[D_8013BE30.activeIndex] = 1;
            D_8013AE88->data18 = 0;
            break;
        }
        if ((s16)func_800198EC(MEMORY_CARD_SLOT_BUFFER_BASE +
                             D_8013BE30.activeIndex * MEMORY_CARD_SLOT_BUFFER_STRIDE) == 1) {
            D_8013BE30.unk_0E[D_8013BE30.activeIndex] = 0;
        }
        break;
    }
}

void startClearedStateValueHistoryTask(void) {
    s16 i;

    func_80014874(updateMemoryCardSlotTask);
    D_8013AE8C->data18 = 0;
    D_8013BE30.unk_00 = 16;
    D_8013BE30.activeIndex = 1;
    for (i = 0; i < 2; i++) {
        D_8013BE30.current[i] = 0;
        D_8013BE30.previous[i] = 0;
        D_8013BE30.unk_0E[i] = 0;
        D_8013BE30.unk_12[i] = 0;
        D_8013BE30.unk_16[i] = 0;
    }
}

void func_800879CC(s16 slot, s32 x, s32 y, s16 depth, s32 index);

void initializeSaveRecordTask(void) {
    s16 i;

    D_8013AE88->data18 = 0;
    D_8013AE98->data1C++;
    if (D_8013AE88->resource14 != 0xFFFF) {
        while (1) {}
    }
    D_8013AE88->resource14 = allocateTaskResource();
    if (D_8013AE88->resource16 != 0xFFFF) {
        while (1) {}
    }
    D_8013AE88->resource16 = allocateTaskResource();
    func_800897D0((SaveRecord *)&D_8013080C);
    func_800897D0(&((SaveRecord *)&D_8013080C)[1]);
    copyGlobalFlagState(&D_8013080C, &D_8013101C);
    mergeSaveRecordFlags(&((SaveRecord *)&D_8013080C)[1], (SaveRecord *)&D_8013101C);
    D_8013B484[0] = (GlobalFlagStatePrefix *)&SAVE_RECORD_WORKSPACE[0];
    D_8013B484[1] = (GlobalFlagStatePrefix *)&SAVE_RECORD_WORKSPACE[1];
    func_800897D0((SaveRecord *)D_8013B484[0]);
    func_800897D0((SaveRecord *)D_8013B484[1]);
    for (i = 0; i < 15; i++) {
        gPlayerResourceTextureSelections.entries[0][i] = D_80123650[i];
        gPlayerResourceTextureSelections.entries[1][i] = D_80123650[i];
    }
    startClearedStateValueHistoryTask();
    setCurrentTaskCallback(updateSaveRecordTask);
}

void updateSaveRecordTask(void) {
    SaveRecordTaskStatus *status;
    RegisteredPanelSlot *panels;
    s16 firstResult;
    s16 secondResult;

    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    status = (SaveRecordTaskStatus *)&D_800D9504[D_8013AE88->resource14];
    if (D_8013AE88->resource16 == 0xFFFF) {
        while (1) {}
    }
    panels = (RegisteredPanelSlot *)&D_800D9504[D_8013AE88->resource16];

    switch (D_8013AE88->data18) {
    case 0:
        setMusicSequenceEnabled(1);
        status->first.flags = 0;
        status->secondResult = 0;
        uploadBufferedTexturePair();
        func_800879CC(0, -136, -96, 6, 1);
        func_800879CC(1, -136, 8, 6, 11);
        D_8013AE88->data18 = 2;
        break;
    case 2:
        if (panels[0].flags || panels[1].flags) {
            break;
        }
        markSecondaryTaskAndClearReference();
        panels[0].command = 3;
        D_8013AE88->data18 = 3;
        break;
    case 3:
        if (panels[0].flags) {
            break;
        }
        if (status->first.flags < 0) {
            goto finish_panels;
        }
        panels[1].command = 3;
        D_8013AE88->data18 = 4;
        break;
    case 4:
        if (panels[1].flags) {
            break;
        }
        if (status->secondResult >= 0) {
            if (panels[0].task->data18 != 26) {
                panels[0].command = 15;
            }
            if (panels[1].task->data18 != 26) {
                panels[1].command = 15;
            }
        }
    finish_panels:
        D_8013AE88->data18 = 5;
        break;
    case 5:
        if ((firstResult = status->first.flags) < 0 ||
            (secondResult = status->secondResult) < 0) {
            D_8013AE98->data18 = 3;
            resetUnlockNotificationStateForMode2();
        } else {
            if (firstResult == 0 || secondResult == 0 || panels[0].flags || panels[1].flags) {
                break;
            }
            if (D_8013AFCD == 1 && D_8013B47C == 0) {
                D_8013AE98->data18 = 30;
            } else {
                D_8013AE98->data18 = 5;
                queueMenuResourceReads(5);
            }
            applyStereoSoundOption();
        }
        D_8013AE88->data18 = 6;
        break;
    case 6:
        panels[0].command = 27;
        panels[1].command = 27;
        D_8013AE88->data18 = 7;
        break;
    case 7:
        if (panels[0].flags || panels[1].flags) {
            break;
        }
        if (D_8013AE98->data18 == 30) {
            configureTaskStateTransition(2, 0, 30);
            D_8013AE88->data18 = 8;
            break;
        }
        goto finish_transition;
    case 8:
        if (D_8013AFCD == 2) {
            D_8013AE98->data18 = 5;
            queueMenuResourceReads(5);
        } else {
            if (func_800707F0(0) != 0) {
                break;
            }
            setMusicSequenceEnabled(0);
            setTaskBackgroundMode(0, 0, 0);
        }
    finish_transition:
        D_8013AE88->data18 = 9;
        break;
    case 9:
    default:
        D_8013AE98->data1C--;
        func_800149A8();
        break;
    }
}

void resetUnlockNotificationStateForMode2(void) {
    if (D_8013AFCD == 2) {
        gUnlockNotificationFlags = 0;
        D_8013B47C = 0;
    }
}

INCLUDE_ASM("asm/nonmatchings/unclassified/menu_8007CC78", func_800879CC);

INCLUDE_ASM("asm/nonmatchings/unclassified/menu_8007CC78", func_80087B84);

INCLUDE_ASM("asm/nonmatchings/unclassified/menu_8007CC78", func_80088EB4);

INCLUDE_ASM("asm/nonmatchings/unclassified/menu_8007CC78", func_800895DC);
