#include "../menu/save_record_tasks_internal.h"
#include "game/io/dat_resources.h"

typedef struct MenuModelResource {
    GsDOBJ2 object;
    GsCOORDINATE2 local;
    SVECTOR localRotation;
    GsCOORDINATE2 world;
    SVECTOR worldRotation;
    s16 unkC0, unkC2, unkC4, unkC6;
} MenuModelResource;

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

StateValueHistory D_8013BE30 __attribute__((section(".bss.state_value_history")));

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

INCLUDE_ASM("asm/nonmatchings/unclassified/menu_8007CC78", func_8007E8F0);

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
        adjustGroupedIndexForMode(D_8013AE88->data1A, &D_80120E90[0].unk00A[1]);
        D_801300AC[0].selection = getGroupedIndexBase(D_80120E90[0].unk00A[1]);
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
    if (D_80120E90[playerIndex].unk009 == 8) {
        result = 2;
    } else if (D_80120E90[playerIndex].unk009 == 9) {
        result = 5;
    } else {
        result = (record->flags_3AC & 0x3800) != 0;
    }
    return result;
}

INCLUDE_ASM("asm/nonmatchings/unclassified/menu_8007CC78", func_80080C64);

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

INCLUDE_ASM("asm/nonmatchings/unclassified/menu_8007CC78", func_80082690);

void reloadTaskPlayerModel(GameTask *task, u16 slot, u16 selection, u16 modelIndex, s16 x, s16 y, s16 z) {
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

INCLUDE_ASM("asm/nonmatchings/unclassified/menu_8007CC78", func_80082CE0);

void sortLitObjectWithDrawArea(GsDOBJ2 *object, u16 index, RECT *bounds) {
    MATRIX matrix;
    GsGetLw(object->coord2, &matrix);
    GsSetLightMatrix(&matrix);
    GsGetLs(object->coord2, &matrix);
    GsSetLsMatrix(&matrix);
    {
        u32 depthIndex = index;
        GsSortObject4(object, &D_800EBFF0[D_8013AF78], (depthIndex & 1) + 1,
                      (u32 *)0x1F800000);
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

INCLUDE_ASM("asm/nonmatchings/unclassified/menu_8007CC78", func_80083180);

INCLUDE_ASM("asm/nonmatchings/unclassified/menu_8007CC78", func_80083574);

static inline RegisteredTaskEntry *getRegisteredBlinkEntry(u16 resource, u32 index) {
    RegisteredTaskEntry *entries = (RegisteredTaskEntry *)&D_800D9504[resource];
    return &entries[(u16)(index / 10)];
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
        entry = getRegisteredBlinkEntry(D_8013AE98->resource14, D_8013AE88->data1E);
    } else {
        if (D_8013AE98->resource16 == 0xFFFF) { while (1) {} }
        entry = getRegisteredBlinkEntry(D_8013AE98->resource16, D_8013AE88->data1E);
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
    setCurrentTaskCallback(func_800843DC);
    D_8013AE88->unk10 |= 2;
}

INCLUDE_ASM("asm/nonmatchings/unclassified/menu_8007CC78", func_800843DC);

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
    adjustGroupedIndexForMode((s16)D_8013AE8C->data1A, &((PlayerSelectionRecord *)D_80120E90)[(s16)player].selectionBytes[3]);
    selection->selection = getGroupedIndexBase(((PlayerSelectionRecord *)D_80120E90)[(s16)player].selectionBytes[3]);
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

INCLUDE_ASM("asm/nonmatchings/unclassified/menu_8007CC78", func_80086584);

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
    func_8006D66C(resource->x / 4096 + 54, resource->y / 4096 + 4,
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

INCLUDE_ASM("asm/nonmatchings/unclassified/menu_8007CC78", func_80087030);

void startClearedStateValueHistoryTask(void) {
    s16 i;

    func_80014874(func_80087030);
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
    setCurrentTaskCallback(func_800875D0);
}

INCLUDE_ASM("asm/nonmatchings/unclassified/menu_8007CC78", func_800875D0);

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
