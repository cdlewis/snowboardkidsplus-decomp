#include "common.h"
#include "game/global_flags.h"
#include "game/graphics.h"
#include "game/game_options.h"
#include "game/linear_random.h"
#include "game/offset_archive.h"
#include "game/music.h"
#include "psx/libsnd.h"
#define TASK_ABSOLUTE_ADDRESSING
#include "game/task.h"
#include "game/race_player.h"
#include "game/movement.h"
#include "game/menu_selection_state.h"
#include "game/player_resource_textures.h"

typedef struct PaletteRotationResource {
    RECT rect;
    u16 original[16];
    u16 rotated[16];
    s16 interval;
    s16 timer;
    s8 first;
    s8 last;
    s8 steps;
} PaletteRotationResource;

typedef struct PaletteEntryResource {
    RECT rect;
    u16 original[16];
    u16 modified[16];
    s8 destination;
    s8 source;
} PaletteEntryResource;

typedef struct PaletteRotationSlot {
    GameTask *task;
    s16 state;
    s16 unk06;
} PaletteRotationSlot;

/* Selection bytes at player offsets 8..12, with the full player stride. */
typedef struct PlayerSelectionRecord {
    GameTask *task;
    u16 playerIndex;
    u8 unk06[2];
    u8 selectionBytes[5];
    u8 unk0D[0x9CF];
} PlayerSelectionRecord;

typedef struct RegisteredPanelSlot {
    GameTask *task;
    s16 command;
    s16 flags;
} RegisteredPanelSlot;

typedef struct GroupedIndexResource {
    u8 unk00[0x48];
    const u8 *values;
    u8 unk4C[0xB4];
} GroupedIndexResource;

typedef struct RegisteredTripletData {
    ByteTriplet *records;
    u8 *indices;
    s16 offsetX;
    s16 offsetY;
} RegisteredTripletData;

typedef struct HalfwordPair {
    s16 unk_00;
    s16 unk_02;
} HalfwordPair;

typedef struct IndexedHalfwordRecord {
    s16 value;
    s16 unk_02;
    s16 unk_04;
} IndexedHalfwordRecord;

typedef struct SelectionTransitionEntry {
    s16 selection;
    s16 previousSelection;
    s16 timer;
} SelectionTransitionEntry;

typedef struct StateValueHistory {
    s32 unk_00;
    s16 activeIndex;
    s16 current[2];
    s16 previous[2];
    s16 unk_0E[2];
    s16 unk_12[2];
    s16 unk_16[2];
} StateValueHistory;

u16 litObjectDrawAreaDepths[4] __attribute__((section(".data.lit_object_draw_area_depths"))) = {
    0x30, 0x20, 0x50, 0x3E
};

HalfwordPair D_8012F13C[16] __attribute__((section(".data"))) = {0};

u16 gTaskShadowedText[14] __attribute__((section(".data.task_shadowed_text"))) = {
    0x001A, 0x001F, 0x001D, 0x0012, 0x8020, 0x8020, 0x000C,
    0x001F, 0x001E, 0x001E, 0x0019, 0x0018, 0, 0
};

StateValueHistory D_8013BE30 __attribute__((section(".bss.state_value_history")));
extern u8 D_8013AFD8 __attribute__((section(".data")));
extern u8 D_8013AFD9 __attribute__((section(".data")));
extern u8 D_8013B47C;
extern u8 D_8013B6B8[1];
extern u8 D_8013B6B9[7];

void func_800897D0(SaveRecord *record);
void mergeSaveRecordFlags(SaveRecord *source, SaveRecord *destination);
void func_800875D0(void);
void func_8007D5C8(void);
void updatePaletteRotationTask(void);
void func_80089B30(void);
void func_80087030(void);
void func_800857C4(void);
void func_800843DC(void);
void func_80084970(void);
void func_800801A0(void);
void func_800815D0(void);
void initializeDualResourceTaskAndResetByteState(void);
void adjustGroupedIndexForMode(s16 mode, u8 *index);
void func_8007F630(void);
void func_80081DF0(void);
void func_800816D8(void);

INCLUDE_ASM("asm/nonmatchings/game/unk_8007CC78", func_8007CC78);

INCLUDE_ASM("asm/nonmatchings/game/unk_8007CC78", func_8007D47C);

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

INCLUDE_ASM("asm/nonmatchings/game/unk_8007CC78", func_8007D5C8);

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

INCLUDE_ASM("asm/nonmatchings/game/unk_8007CC78", func_8007E298);

INCLUDE_ASM("asm/nonmatchings/game/unk_8007CC78", func_8007E8CC);

INCLUDE_ASM("asm/nonmatchings/game/unk_8007CC78", func_8007E8F0);

INCLUDE_ASM("asm/nonmatchings/game/unk_8007CC78", func_8007EE68);

void clearHalfwordPairTable(void) {
    s16 i;

    for (i = 0; i < 16; i++) {
        D_8012F13C[i].unk_00 = 0;
        D_8012F13C[i].unk_02 = 0;
    }
}

INCLUDE_ASM("asm/nonmatchings/game/unk_8007CC78", func_8007F46C);

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

INCLUDE_ASM("asm/nonmatchings/game/unk_8007CC78", func_8007F630);

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

INCLUDE_ASM("asm/nonmatchings/game/unk_8007CC78", func_800801A0);

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

INCLUDE_ASM("asm/nonmatchings/game/unk_8007CC78", func_80080C64);

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

INCLUDE_ASM("asm/nonmatchings/game/unk_8007CC78", func_800815D0);

INCLUDE_ASM("asm/nonmatchings/game/unk_8007CC78", func_800816D8);

INCLUDE_ASM("asm/nonmatchings/game/unk_8007CC78", func_80081AC8);

INCLUDE_ASM("asm/nonmatchings/game/unk_8007CC78", func_80081BAC);

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

INCLUDE_ASM("asm/nonmatchings/game/unk_8007CC78", func_80081DF0);

INCLUDE_ASM("asm/nonmatchings/game/unk_8007CC78", func_80082690);

INCLUDE_ASM("asm/nonmatchings/game/unk_8007CC78", func_80082A30);

INCLUDE_ASM("asm/nonmatchings/game/unk_8007CC78", func_80082CE0);

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

INCLUDE_ASM("asm/nonmatchings/game/unk_8007CC78", func_80083180);

INCLUDE_ASM("asm/nonmatchings/game/unk_8007CC78", func_80083574);

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

INCLUDE_ASM("asm/nonmatchings/game/unk_8007CC78", func_80083C3C);

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

INCLUDE_ASM("asm/nonmatchings/game/unk_8007CC78", func_800843DC);

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

INCLUDE_ASM("asm/nonmatchings/game/unk_8007CC78", func_80084970);

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

INCLUDE_ASM("asm/nonmatchings/game/unk_8007CC78", func_800857C4);

INCLUDE_ASM("asm/nonmatchings/game/unk_8007CC78", func_80086584);

INCLUDE_ASM("asm/nonmatchings/game/unk_8007CC78", func_8008690C);

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

INCLUDE_ASM("asm/nonmatchings/game/unk_8007CC78", func_80086C34);

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

INCLUDE_ASM("asm/nonmatchings/game/unk_8007CC78", func_80087030);

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
    D_8013B484[0] = (GlobalFlagStatePrefix *)0x801E0000;
    D_8013B484[1] = (GlobalFlagStatePrefix *)0x801E0408;
    func_800897D0((SaveRecord *)D_8013B484[0]);
    func_800897D0((SaveRecord *)D_8013B484[1]);
    for (i = 0; i < 15; i++) {
        gPlayerResourceTextureSelections.entries[0][i] = D_80123650[i];
        gPlayerResourceTextureSelections.entries[1][i] = D_80123650[i];
    }
    startClearedStateValueHistoryTask();
    setCurrentTaskCallback(func_800875D0);
}

INCLUDE_ASM("asm/nonmatchings/game/unk_8007CC78", func_800875D0);

void resetUnlockNotificationStateForMode2(void) {
    if (D_8013AFCD == 2) {
        gUnlockNotificationFlags = 0;
        D_8013B47C = 0;
    }
}

INCLUDE_ASM("asm/nonmatchings/game/unk_8007CC78", func_800879CC);

INCLUDE_ASM("asm/nonmatchings/game/unk_8007CC78", func_80087B84);

INCLUDE_ASM("asm/nonmatchings/game/unk_8007CC78", func_80088EB4);

INCLUDE_ASM("asm/nonmatchings/game/unk_8007CC78", func_800895DC);

void repeatByteStore(u8 *destination, s32 value, s32 count) {
    s16 i;

    /* The original loop writes repeatedly to the same byte. */
    for (i = 0; i < count; i++) {
        *destination = value;
    }
}

INCLUDE_ASM("asm/nonmatchings/game/unk_8007CC78", func_800897D0);

void copyGlobalFlagState(GlobalFlagStatePrefix *source, GlobalFlagStatePrefix *destination) {
    s16 i;

    destination->flags = source->flags;
    for (i = 0; i < 10; i++) {
        i[destination->masks] = i[source->masks];
    }
    destination->combinedMask = source->combinedMask;
    destination->unk_3E4 = source->unk_3E4;
}

void mergeSaveRecordFlags(SaveRecord *source, SaveRecord *destination) {
    s16 i;
    destination->flags_3AC |= source->flags_3AC;
    for (i = 0; i < 10; i++) {
        i[destination->flags_3B0] |= i[source->flags_3B0];
    }
    destination->flags_3D8 |= source->flags_3D8;
}

void copySaveRecordAndMergeFlags(void) {
    *(SaveRecord *)&D_8013101C = *(SaveRecord *)&D_8013080C;
    mergeSaveRecordFlags(&((SaveRecord *)&D_8013080C)[1], (SaveRecord *)&D_8013101C);
}

INCLUDE_ASM("asm/nonmatchings/game/unk_8007CC78", func_80089A0C);

INCLUDE_ASM("asm/nonmatchings/game/unk_8007CC78", func_80089A98);

u32 calculateByteSum(const u8 *data, u32 size) {
    u32 sum;
    u32 i;

    sum = 0;
    for (i = 0; i < size; i++) {
        sum += *data++;
    }
    return sum;
}

INCLUDE_ASM("asm/nonmatchings/game/unk_8007CC78", func_80089B30);

void startStateValueHistoryTask(void) {
    s16 i;
    func_80014874(func_80089B30);
    D_8013AE8C->data18 = 0;
    D_8013BE30.unk_00 = 16;
    D_8013BE30.activeIndex = 1;
    for (i = 0; i < 2; i++) {
        D_8013BE30.current[i] = 0;
        D_8013BE30.previous[i] = 0;
        D_8013BE30.unk_0E[i] = 0;
        D_8013BE30.unk_12[i] = 0;
        D_8013BE30.unk_16[i] = 1;
    }
}

INCLUDE_ASM("asm/nonmatchings/game/unk_8007CC78", func_80089EC0);

INCLUDE_ASM("asm/nonmatchings/game/unk_8007CC78", func_8008A058);

INCLUDE_ASM("asm/nonmatchings/game/unk_8007CC78", func_8008B178);

void drawTaskShadowedText(void) {
    Movement2D *resource;
    s32 x, y;
    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (Movement2D *)&D_800D9504[D_8013AE88->resource14];
    x = resource->x / 4096;
    y = resource->y / 4096;
    func_8006EC84(x - 48, y + 24, 12, gTaskShadowedText, 0, -1,
                  &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
    setSpriteColor(0, 0, 0);
    func_8006EC84(x - 47, y + 25, 12, gTaskShadowedText, 0, -1,
                  &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
    func_8006EC84(x - 46, y + 25, 12, gTaskShadowedText, 0, -1,
                  &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
}

s16 getSaveRecordMaskTier(s16 recordIndex, s16 maskIndex) {
    s16 result = 0;
    s16 i = 3;
    SaveRecord *records;

    for (; i >= 0; i--) {
        records = (SaveRecord *)&D_8013080C;
        if (records[recordIndex].flags_3B0[maskIndex] & (0x1000000 << i)) {
            result = i + 1;
            break;
        }
    }
    return result;
}

void setSaveRecordMaskTier(s16 recordIndex, s16 maskIndex, s16 tier) {
    s16 i;
    SaveRecord *records = (SaveRecord *)&D_8013080C;

    records[recordIndex].flags_3B0[maskIndex] &= 0xF0FFFFFF;
    for (i = 3; i >= 0; i--) {
        if (i < tier) {
            records[recordIndex].flags_3B0[maskIndex] |= 0x1000000 << i;
        }
    }
    rebuildCombinedStateMask((GlobalFlagStatePrefix *)&((SaveRecord *)&D_8013080C)[recordIndex]);
    copySaveRecordAndMergeFlags();
}

s16 getStateMaskTier(s16 stateIndex, s16 maskIndex) {
    s16 result = 0;
    s16 i;
    u32 mask;

    mask = D_8013B484[stateIndex]->masks[maskIndex];
    for (i = 3; i >= 0; i--) {
        if (mask & (0x1000000 << i)) {
            result = i + 1;
            break;
        }
    }
    return result;
}

s16 getSaveRecordFlagPair(s16 recordIndex, s16 maskIndex, s16 pairIndex) {
    u32 first;
    u32 second;
    s16 result;
    SaveRecord *records = (SaveRecord *)&D_8013080C;

    first = records[recordIndex].flags_3B0[maskIndex] & (1U << (pairIndex * 2));
    second = records[recordIndex].flags_3B0[maskIndex] & (2U << (pairIndex * 2));
    if (first && second) {
        result = 3;
    } else if (second) {
        result = 2;
    } else if (first) {
        result = 1;
    } else {
        result = 0;
    }
    return result;
}

void setSaveRecordFlagPair(s16 recordIndex, s16 maskIndex, s16 pairIndex, s16 value) {
    GlobalFlagStatePrefix *state = &D_8013080C;

    ((SaveRecord *)state)[recordIndex].flags_3B0[maskIndex] &= ~(3 << (pairIndex * 2));
    if (value == 1 || value == 3) {
        ((SaveRecord *)&D_8013080C)[recordIndex].flags_3B0[maskIndex] |= 1 << (pairIndex * 2);
    }
    if (value == 2 || value == 3) {
        GlobalFlagStatePrefix *secondState = &D_8013080C;
        ((SaveRecord *)secondState)[recordIndex].flags_3B0[maskIndex] |= 2 << (pairIndex * 2);
    }
    rebuildCombinedStateMask((GlobalFlagStatePrefix *)&((SaveRecord *)&D_8013080C)[recordIndex]);
    copySaveRecordAndMergeFlags();
}

s32 getStateMaskPair(s16 stateIndex, s16 maskIndex, s16 pairIndex) {
    u32 mask = D_8013B484[stateIndex]->masks[maskIndex];
    u32 low = mask & (1 << (pairIndex * 2));
    u32 high = mask & (2 << (pairIndex * 2));
    return low && high ? 3 : high ? 2 : low ? 1 : 0;
}

void rebuildCombinedStateMask(GlobalFlagStatePrefix *state) {
    s16 i;

    state->combinedMask = 0;
    for (i = 0; i < 10; i++) {
        state->combinedMask |= i[state->masks];
    }
}

INCLUDE_ASM("asm/nonmatchings/game/unk_8007CC78", func_8008C290);

u32 getGlobalFlagMask(s16 bitIndex) {
    return D_801313C8[0] & (1U << bitIndex);
}

void updatePairedStateFlag(s16 index) {
    if (index < 0) {
        D_8013080C.flags &= ~3U;
        D_8013101C.flags &= ~3U;
    } else {
        D_8013080C.flags |= 1U << index;
        D_8013101C.flags |= 1U << index;
    }
}

void setPairedStateIndexedFlag(s16 index) {
    GlobalFlagStatePrefix *first = &D_8013080C;
    u32 mask = 0x800U << (index + 3);
    GlobalFlagStatePrefix *second = &D_8013101C;

    first->flags |= mask;
    second->flags |= mask;
}

s32 getModeFilteredGameOptionFlag2(void) {
    if (D_8013AFD8 != 0) {
        return (u32)(D_8013AFD8 - 7) < 2;
    }
    if (D_8013AFD9 != 0) {
        return 0;
    }
    if (D_8013AFCD < 2) {
        return 1;
    }
    return gGameOptionFlags & 4;
}

s32 getStateAdjustedGameOptionFlag1(void) {
    if (D_8013AFD8 != 0) {
        return (u32)(D_8013AFD8 - 7) < 2;
    }
    if (D_8013AFD9 != 0) {
        return 0;
    }
    if (D_8013AFCD < 2) {
        return 1;
    }
    return gGameOptionFlags & 2;
}
