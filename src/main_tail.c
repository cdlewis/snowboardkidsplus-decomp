#include "common.h"
#include "main_tail_sound_slots.h"
#include "game/controller.h"
#include "game/controller_effects.h"
#include "game/global_flags.h"
#define RACE_FINISH_ABSOLUTE_ADDRESSING
#define SELECTED_RECORDS_ABSOLUTE_ADDRESSING
#include "game/selected_records.h"
#include "game/cd_queue.h"
#include "game/byte_search.h"
#include "game/game_options.h"
#define GAME_LOOP_ABSOLUTE_ADDRESSING
#include "game/game_loop.h"
#include "game/graphics.h"
#define MUSIC_ABSOLUTE_ADDRESSING
#include "game/music.h"
#include "psx/libsnd.h"
#define RACE_PLAYER_ABSOLUTE_ADDRESSING
#define TASK_MODE_STATE_ABSOLUTE_ADDRESSING
#define TASK_ABSOLUTE_ADDRESSING
#define TASK_FLAGS_ABSOLUTE_ADDRESSING
#include "game/race_player.h"
#include "game/race_camera.h"
#include "game/text_entries.h"
#include "game/task.h"
#include "game/movement.h"
#include "psx/libgpu.h"
#include "psx/libgs.h"
#include "game/menu_selection_state.h"
#include "game/coordinate.h"
#include "game/linear_random.h"

#define INDEXED_SECTOR_READ_DESTINATION ((void *)0x801F2080)

typedef struct PaletteBurstResource {
    s16 x, y, z, unk06;
    s16 rotation, unk0A;
    s32 age;
    u8 r, g, b;
} PaletteBurstResource;
typedef struct GridMarkerState {
    u8 unk00[0x30];
    u16 state;
    s16 selectedIndex;
    u8 unk34[0x1C];
    u16 depth;
    u8 unk52[0x10];
    u8 red, green, blue;
} GridMarkerState;

typedef struct RegisteredPanelResource {
    s16 unk00, unk02, unk04, unk06, unk08;
    s16 x, y, textX, textY;
    const u16 *text;
    s16 value, color, mode;
    const u16 *content;
    s32 depth, index;
} RegisteredPanelResource;

/* Menu descriptors have a 0x14-byte stride in the callers' tables. */
typedef struct ByteMappingDescriptor {
    const u8 *mapping;
    u8 unk04[2];
    u8 count;
    u8 unk07[13];
} ByteMappingDescriptor;

typedef struct ModeSectorReadState {
    u8 unk00[0xA];
    s16 sectorIndex;
    u8 unk0C[0x2C];
    s16 mode;
    u8 unk3A[0xC6];
} ModeSectorReadState;
typedef struct TextSequenceResource {
    u8 unk00[0xA];
    u16 unk0A;
    u16 entryIndex;
    u8 unk0E[6];
    s32 x, y, velocity;
    u8 unk20[0xC];
    s32 displacement;
    u16 progress;
    u16 duration;
    s16 state;
    u8 intensity;
    u8 unk37;
    s16 mode;
    u16 unk3A;
    u8 unk3C[0xC4];
} TextSequenceResource;

typedef struct SplitBoundsResource {
    u8 unk00[0x1C];
    s16 unk1C;
    u8 unk1E[6];
    s16 unk24;
} SplitBoundsResource;

typedef struct SplitSelectionBoundsConfigs {
    u8 entries[10][3];
    u8 padding[2];
} SplitSelectionBoundsConfigs;

typedef struct UnlockMenuState {
    s16 selection, previousSelection;
    s32 unk04[7];
    s32 unk20[7];
    s32 unk3C[7];
    s32 unk58[7];
    s16 unk74[4];
    u8 unk7C[0x84];
} UnlockMenuState;

typedef struct NumberAdjustmentState {
    s16 value;
    s16 countdown;
    s16 active;
    u8 unk06[0x1A];
    s32 unk20[3];
    s32 unk2C[3];
    u8 unk38[0xC8];
} NumberAdjustmentState;

typedef struct TextControlState {
    s16 unk00;
    s16 unk02;
    s32 unk04;
    s32 unk08;
    s32 unk0C;
    u8 unk10[8];
    s16 unk18[5];
} TextControlState;
typedef struct TextResourceState {
    TextEntry *currentEntry;
    u32 unk04[5];
    s16 unk18[5];
} TextResourceState;
extern s16 D_8013B10C __attribute__((section(".data")));
extern u8 D_8013B55E;
extern u8 D_8013B527 __attribute__((section(".data")));

typedef struct MusicOwnerState {
    u8 unk00[0x24];
    u32 tick;
    u8 unk28[0xC];
    GameTask *musicTask;
    u8 unk38[8];
    u32 completionTick;
} MusicOwnerState;

typedef struct TextEntryListHeader {
    TextEntry *freeEntries;
    TextEntry *activeEntries;
} TextEntryListHeader;

u16 gCurrentTaskPanelText[8] __attribute__((section(".data.current_task_panel_text"))) = {
    0xFA, 0x94, 0xB1, 0x9F, 0x98, 0x31, 0, 0
};

const u16 gTaskPanel144x48Text[8] __attribute__((section(".data.task_panel_144x48_text"))) = {
    0xF4, 0xC1, 0x96, 0xB1, 0x9F, 0x98, 0x31, 0
};

const u16 gTaskPlayerValuePanelLabel[6] __attribute__((section(".data.task_player_value_panel_label"))) = {
    0x18, 0x157, 0x2D, 0x8020, 0xA8, 0
};
const u16 gTaskPlayerValuePanelText[8] __attribute__((section(".data.task_player_value_panel_text"))) = {
    0xF4, 0xC1, 0x96, 0xB1, 0x9E, 0xA2, 0x33, 0
};

u32 gPlayerPaletteTransitionPixels[8] __attribute__((section(".data.player_palette_transition_pixels"))) = {0};
ResourceSectorRange gGridResourceSecondSectorRanges[10][5] __attribute__((section(".data.grid_resource_sector_ranges"))) = {
    {{0x4CA, 8}, {0x4DA, 8}, {0x4EA, 8}, {0x4FA, 8}, {0x50A, 8}},
    {{0x51A, 8}, {0x529, 8}, {0x538, 8}, {0x548, 8}, {0x558, 8}},
    {{0x567, 8}, {0x577, 8}, {0x587, 8}, {0x597, 8}, {0x5A7, 8}},
    {{0x5B6, 8}, {0x5C5, 8}, {0x5D4, 8}, {0x5E3, 8}, {0x5F2, 8}},
    {{0x601, 8}, {0x610, 8}, {0x61F, 8}, {0x62E, 8}, {0x63D, 8}},
    {{0x64C, 8}, {0x65C, 8}, {0x66C, 8}, {0x67C, 8}, {0x68C, 8}},
    {{0x69C, 8}, {0x6AC, 8}, {0x6BC, 8}, {0x6CC, 8}, {0x6DC, 8}},
    {{0x6EC, 8}, {0x6FB, 8}, {0x70A, 8}, {0x719, 8}, {0x728, 8}},
    {{0x737, 8}, {0x745, 8}, {0x753, 8}, {0x761, 8}, {0x76F, 8}},
    {{0x77D, 8}, {0x78D, 8}, {0x79D, 8}, {0x7AD, 8}, {0x7BD, 8}}
};

ResourceSectorRange gGridResourceFirstSectorRanges[10][5] __attribute__((section(".data.grid_resource_sector_ranges"))) = {
    {{0x4D2, 8}, {0x4E2, 8}, {0x4F2, 8}, {0x502, 8}, {0x512, 8}},
    {{0x522, 7}, {0x531, 7}, {0x540, 8}, {0x550, 8}, {0x560, 7}},
    {{0x56F, 8}, {0x57F, 8}, {0x58F, 8}, {0x59F, 8}, {0x5AF, 7}},
    {{0x5BE, 7}, {0x5CD, 7}, {0x5DC, 7}, {0x5EB, 7}, {0x5FA, 7}},
    {{0x609, 7}, {0x618, 7}, {0x627, 7}, {0x636, 7}, {0x645, 7}},
    {{0x654, 8}, {0x664, 8}, {0x674, 8}, {0x684, 8}, {0x694, 8}},
    {{0x6A4, 8}, {0x6B4, 8}, {0x6C4, 8}, {0x6D4, 8}, {0x6E4, 8}},
    {{0x6F4, 7}, {0x703, 7}, {0x712, 7}, {0x721, 7}, {0x730, 7}},
    {{0x73F, 6}, {0x74D, 6}, {0x75B, 6}, {0x769, 6}, {0x777, 6}},
    {{0x785, 8}, {0x795, 8}, {0x7A5, 8}, {0x7B5, 8}, {0x7C5, 8}}
};

ResourceSectorRange gMode1SectorRanges[10] __attribute__((section(".data.indexed_sector_ranges"))) = {
    {0x236E, 1},
    {0x236F, 1},
    {0x2370, 1},
    {0x2371, 1},
    {0x2372, 1},
    {0x2373, 1},
    {0x2374, 1},
    {0x2375, 1},
    {0x2376, 1},
    {0x2377, 1}
};
ResourceSectorRange gOtherModeSectorRanges[11] __attribute__((section(".data.indexed_sector_ranges"))) = {
    {0x2363, 1},
    {0x2364, 1},
    {0x2366, 1},
    {0x2365, 1},
    {0x2367, 1},
    {0x2368, 1},
    {0x236A, 1},
    {0x2369, 1},
    {0x236C, 1},
    {0x236B, 1},
    {0x236D, 1}
};

POLY_F3 gNumberAdjustmentArrow __attribute__((section(".data.number_adjustment_arrow"))) = {
    4U << 24, 0, 0, 0, POLY_F3_CODE, 0, 0, 0, 0, 0, 0
};

SplitSelectionBoundsConfigs gSplitSelectionBoundsConfigs __attribute__((section(".data.split_selection_bounds_configs"))) = {{
    {60, 120, 4}, {30, 180, 6}, {30, 180, 6}, {60, 120, 4},
    {30, 180, 6}, {30, 180, 6}, {60, 120, 4}, {30, 180, 6},
    {60, 120, 4}, {60, 120, 4}
}, {0, 0}};

u16 playerModelDrawAreaDepths[4] __attribute__((section(".data.player_model_draw_area_depths"))) = {
    0x70, 0x40, 0x20, 0
};

u8 D_8013B514[3] __attribute__((section(".sdata.four_bounds_task_flag"))) = {0x82, 0x66, 0};
u8 D_8013B517 __attribute__((section(".sdata.four_bounds_task_flag"))) = 0;

extern s16 D_8013B560;
extern u8 D_8013B562;
extern u8 D_8013AF3A __attribute__((section(".data")));


void func_8009488C(void);
void func_800187B8(u8 voice);

void func_8008CD98(void);
void func_8008FA8C(void);
void func_8008D200(void);
void func_8008DDC0(void);
void func_8008EC50(void);
void func_80090450(void);
void func_80092090(void);
void func_80097FF4(void);
void func_80099880(void);
void func_800A0FDC(void);
void func_800A1774(TextSequenceResource *resource);
void func_80092A50(void);
void prepareRaceResourcesIfSelectionChanged(void);
void SetFogNearFar(s32 near, s32 far, s32 h);
void func_800A0EE8(void);
void queueTaskModeSectorRead(void);
void queueModeIndexedSectorRead(s16 mode, s16 index);
void initializeControllerPlaybackRaceTask(void);
void func_800A5AFC(void);
void initializeTextPlaybackTask(void);
void func_8009B700(void);
void func_8009BDB8(void);
void func_8009CA90(void);
void func_8009F9B8(void);
void func_8009F224(void);

void func_8002FA58(s16 resource);
void func_800A42CC(void);
void func_800A499C(void);
void preparePlayerResourcesTask(void);
void initializeRaceTaskState(void);
void func_8001B858(void);
void func_800A1F50(void);
void func_800A4C28(void);
void resetUnlockMenuOffsets(UnlockMenuState *state);
void func_800A525C(void);
void func_800A53BC(void);
void func_800A6A8C(void);
void func_800A624C(void);
void func_8009D354(void);
void func_800A05C0(void);
void startUnlockResourceLoad(void);
void prepareUnlockSelectionResume(void);
void func_8003D940(void);
void func_8003EE70(void);
void updateTaskViewAndCheckPlayerLimit(void);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_8008C708);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_8008C8BC);

static inline RegisteredTaskEntry *getRegisteredPanelEntry(u32 resource, s32 index) {
    RegisteredTaskEntry *entries = (RegisteredTaskEntry *)&D_800D9504[resource];
    return &entries[index / 10];
}

void createRegisteredDetailedTextPanelTask(s16 x, s16 y, s16 textX, s16 textY,
                                   const u16 *text, s32 value, s32 color, s32 mode,
                                   const u16 *content, s32 depth, s32 index) {
    RegisteredPanelResource *resource;
    RegisteredTaskEntry *entry;
    func_80014874(func_8008CD98);
    D_8013AE8C->resource14 = allocateTaskResource();
    resource = (RegisteredPanelResource *)&D_800D9504[D_8013AE8C->resource14];
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

INCLUDE_ASM("asm/nonmatchings/main_tail", func_8008CD98);

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

INCLUDE_ASM("asm/nonmatchings/main_tail", func_8008D200);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_8008D958);

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

INCLUDE_ASM("asm/nonmatchings/main_tail", func_8008DDC0);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_8008E66C);

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

INCLUDE_ASM("asm/nonmatchings/main_tail", func_8008EC50);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_8008F474);

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

INCLUDE_ASM("asm/nonmatchings/main_tail", func_8008FA8C);

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

INCLUDE_ASM("asm/nonmatchings/main_tail", func_80090450);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_80091318);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_800916F4);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_80091B80);

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

INCLUDE_ASM("asm/nonmatchings/main_tail", func_80092090);

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

INCLUDE_ASM("asm/nonmatchings/main_tail", func_80092A50);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_800937BC);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_80093A20);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_80093A54);

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

INCLUDE_ASM("asm/nonmatchings/main_tail", func_8009488C);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_80095674);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_80095A58);

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

INCLUDE_ASM("asm/nonmatchings/main_tail", func_80096238);

void queueGridResourceSectorReads(s16 slot, s16 row, s16 column) {
    if (row < 10) {
        queueCdSectorCommand(0, gGridResourceFirstSectorRanges[row][column].sector,
                            gGridResourceFirstSectorRanges[row][column].sectorCount,
                            (u8 *)0x801BC800 + slot * 0x4000, 0);
        queueCdSectorCommand(0, gGridResourceSecondSectorRanges[row][column].sector,
                            gGridResourceSecondSectorRanges[row][column].sectorCount,
                            (u8 *)0x801D0800 + slot * 0x4000, 0);
    } else {
        queueCdSectorCommand(0, 0xC89, 1, (u8 *)0x801BC800 + slot * 0x4000, 0);
    }
}

INCLUDE_ASM("asm/nonmatchings/main_tail", func_80096590);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_80096AB0);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_80096EC8);

void sortPlayerModelWithDrawArea(u16 playerIndex, u16 depthIndex, RECT *bounds) {
    MATRIX matrix;
    RacePlayer *player = &D_80120E90[playerIndex];
    GsDOBJ2 *object = player->modelObjects;
    s16 i;
    s16 count;
    D_8013B0D8 = player;
    count = player->modelPartCount;
    for (i = 0; i < count; i++, object++) {
        GsGetLw(object->coord2, &matrix);
        GsSetLightMatrix(&matrix);
        GsGetLs(object->coord2, &matrix);
        GsSetLsMatrix(&matrix);
        GsSortObject4(object, &D_800EBFF0[D_8013AF78], depthIndex, (u32 *)0x1F800000);
    }
    if (bounds->w >= 0 && bounds->h >= 0) {
        addDrawAreaPacket(bounds->x, bounds->y, bounds->w, bounds->h,
                         &D_800EBFF0[D_8013AF78], playerModelDrawAreaDepths[depthIndex]);
        addDrawAreaPacket(0, 0, 320, 240, &D_800EBFF0[D_8013AF78], 14);
    }
}

void loadCurrentPlayerModelPose(const s16 *values) {
    RacePlayer *player;
    s16 i;
    initCurrentPlayerModelParts();
    player = D_8013B0D8;
    for (i = 0; i < player->modelPartCount; i++) {
        player->unk5BC[i].x = *values++;
        player->unk5BC[i].y = *values++;
        player->unk5BC[i].z = *values++;
    }
    {
        RacePlayer *translationPlayer = D_8013B0D8;
        translationPlayer->unk62C[0].x = (*values + 2) / 4;
        values++;
        translationPlayer->unk62C[0].y = (*values + 2) / 4;
        values++;
        translationPlayer->unk62C[0].z = (*values + 2) / 4;
        values++;
        translationPlayer->unk62C[1].z = -(*values + 2) / 4;
        values++;
        translationPlayer->unk62C[1].y = (*values + 2) / 4;
        values++;
        translationPlayer->unk62C[1].x = (*values + 2) / 4;
        values++;
    }
    setCurrentPlayerFinalPartTranslations(values);
    updateCurrentPlayerModelPartCoordinates();
}

void setCurrentPlayerFinalPartTranslations(const s16 *values) {
    RacePlayer *player = D_8013B0D8;
    s32 index = player->modelPartCount - 2;
    player->unk62C[index].x = *values++;
    player->unk62C[index].y = *values++;
    player->unk62C[index].z = *values++;
    index = player->modelPartCount - 1;
    player->unk62C[index].x = *values++;
    player->unk62C[index].y = *values++;
    player->unk62C[index].z = *values++;
}

void updateCurrentPlayerModelPartCoordinates(void) {
    s16 i;
    u8 parent;
    for (i = 0; i < D_8013B0D8->modelPartCount; i++) {
        parent = D_8013B0D8->unk13E[i];
        RotMatrix((SVECTOR *)&D_8013B0D8->unk5BC[i], &D_8013B0D8->model.modelCoordinates[i].coord);
        if (i == 0) {
            D_8013AF90->vx = 0;
            D_8013AF90->vy = 0;
            D_8013AF90->vz = 0;
            ApplyMatrix(&D_8013B0D8->model.modelCoordinates[0].coord, D_8013AF90, D_8013AF88);
            D_8013B0D8->model.modelCoordinates[0].coord.t[0] = D_8013AF88->vx + D_8013B0D8->unk62C[0].x;
            D_8013B0D8->model.modelCoordinates[0].coord.t[1] = D_8013AF88->vy + D_8013B0D8->unk62C[0].y;
            D_8013B0D8->model.modelCoordinates[0].coord.t[2] = D_8013AF88->vz + D_8013B0D8->unk62C[0].z;
        } else {
            ApplyMatrix(&D_8013B0D8->model.modelCoordinates[parent].coord, (SVECTOR *)&D_8013B0D8->unk62C[i], &D_800EB22C);
            D_8013B0D8->model.modelCoordinates[i].coord.t[0] = D_800EB22C.vx + D_8013B0D8->model.modelCoordinates[parent].coord.t[0];
            D_8013B0D8->model.modelCoordinates[i].coord.t[1] = D_800EB22C.vy + D_8013B0D8->model.modelCoordinates[parent].coord.t[1];
            D_8013B0D8->model.modelCoordinates[i].coord.t[2] = D_800EB22C.vz + D_8013B0D8->model.modelCoordinates[parent].coord.t[2];
        }
        func_8001E3E4((SVECTOR *)&D_8013B0D8->unk5BC[i], &D_8013B0D8->model.modelCoordinates[i]);
    }
}

INCLUDE_ASM("asm/nonmatchings/main_tail", func_80097858);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_80097C2C);

void initializeCountedSplitSelectionBoundsTask(void) {
    D_8013AE98->data1C++;
    D_8013AE88->data18 = 0;
    D_8013AE88->data1A = 0;
    D_8013AE88->data1C = 0;
    if (D_8013AE88->resource14 != 0xFFFF) { for (;;) {} }
    D_8013AE88->resource14 = allocateTaskResource();
    if (D_8013AE88->resource16 != 0xFFFF) { for (;;) {} }
    D_8013AE88->resource16 = allocateTaskResource();
    setSecondaryDisplayTaskMode(2);
    {
        RECT *bounds = &D_801300B8[0];
        bounds->x = 160;
        bounds->y = 0;
        bounds->w = 160;
        bounds->h = 240;
    }
    {
        RECT *bounds = &D_801300B8[1];
        bounds->x = 160;
        bounds->y = 0;
        bounds->w = 160;
        bounds->h = 0;
    }
    {
        RECT *bounds = &D_801300B8[2];
        bounds->x = 24;
        bounds->y = 132;
        bounds->w = 120;
        bounds->h = 84;
    }
    {
        RECT *bounds = &D_801300B8[3];
        bounds->x = 0;
        bounds->y = 0;
        bounds->w = -1;
        bounds->h = -1;
    }
    setCurrentTaskCallback(func_80097FF4);
    D_8013AE88->unk10 |= 2;
}

INCLUDE_ASM("asm/nonmatchings/main_tail", func_80097FF4);

void updateSplitSelectionBoundsTransition(void) {
    SplitBoundsResource *movement;
    u8 index;
    s16 height;
    if (D_8013AE88->resource16 == 0xFFFF) { for (;;) {} }
    movement = (SplitBoundsResource *)&D_800D9504[D_8013AE88->resource16];
    index = D_80120E90[(s16)D_8013AE88->data1A].unk009;
    height = D_801300B8[0].h;
    switch (D_8013AE88->data18) {
    case 19:
        {
            RECT *bounds = &D_801300B8[1];
            bounds->x = 160;
            bounds->y = gSplitSelectionBoundsConfigs.entries[index][0];
            bounds->w = 160;
            bounds->h = gSplitSelectionBoundsConfigs.entries[index][1];
        }
        {
            RECT *bounds = &D_801300B8[0];
            bounds->x = 160;
            bounds->y = gSplitSelectionBoundsConfigs.entries[index][0];
            bounds->w = 160;
            bounds->h = 0;
        }
        movement->unk1C = 13;
        playSoundEffect(1, 0x102, 32);
        D_8013AE88->data18 = 20;
        break;
    case 20:
        height += gSplitSelectionBoundsConfigs.entries[index][2];
        if (height <= gSplitSelectionBoundsConfigs.entries[index][1]) {
            {
                RECT *bounds = &D_801300B8[1];
                bounds->x = 160;
                bounds->y = gSplitSelectionBoundsConfigs.entries[index][0] + height;
                bounds->w = 160;
                bounds->h = gSplitSelectionBoundsConfigs.entries[index][1] - height;
            }
            {
                RECT *bounds = &D_801300B8[0];
                bounds->x = 160;
                bounds->y = gSplitSelectionBoundsConfigs.entries[index][0];
                bounds->w = 160;
                bounds->h = height;
            }
        } else {
            movement->unk24 = 13;
            D_8013AE88->data18 = 21;
        }
        break;
    case 21:
        D_8013AE88->data18 = 16;
        setCurrentTaskCallback(func_80097FF4);
        break;
    }
}

void drawCurrentTaskSpriteB7(void) {
    Movement2D *position;
    if (D_8013AE88->resource14 == 0xFFFF) {
        for (;;) {}
    }
    position = (Movement2D *)&D_800D9504[D_8013AE88->resource14];
    if (D_8013AE88->data1A != 0) {
        func_8006C998(position->x / 4096 + 8, position->y / 4096 - 22,
                      0xB7, 0, -1, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
    }
}

void drawCurrentTaskFramedSprite20B(void) {
    Movement2D *position;
    if (D_8013AE88->resource14 == 0xFFFF) { for (;;) {} }
    position = (Movement2D *)&D_800D9504[D_8013AE88->resource14];
    setTexturedQuadColor(128, 128, 128);
    setSpriteColor(128, 128, 128);
    drawTexturedFrameStyle9(position->x / 4096 - 140, position->y / 4096 + 8,
                           128, 92, 5, 1, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
    func_8006C998(position->x / 4096 - 136, position->y / 4096 + 12,
                  0x20B, 1, 0, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
}

void drawCurrentTaskTextPanel144x48(void) {
    Movement2D *position;
    s32 x, y;
    if (D_8013AE88->resource14 == 0xFFFF) { for (;;) {} }
    position = (Movement2D *)&D_800D9504[D_8013AE88->resource14];
    x = position->x / 4096;
    y = position->y / 4096;
    setSpriteColor(128, 128, 128);
    setTexturedQuadColor(128, 128, 128);
    func_8006EF30((s16)(x + 16), (s16)(y + 12), 10, gTaskPanel144x48Text, 8, -1,
                 &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
    drawDefaultTexturedPanel(x, y, 144, 48, 5, 0,
                            &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
}

void drawCurrentTaskPlayerValuePanel(void) {
    Movement2D *position;
    s32 x, y, value;
    if (D_8013AE88->resource14 == 0xFFFF) { for (;;) {} }
    position = (Movement2D *)&D_800D9504[D_8013AE88->resource14];
    x = position->x / 4096;
    y = position->y / 4096;
    value = D_80120E90[(s16)D_8013AE98->data1A].unk00A[0];
    setSpriteColor(128, 128, 128);
    setTexturedQuadColor(128, 128, 128);
    func_8006EF30((s16)(x + 8), (s16)(y + 4), 10, gTaskPlayerValuePanelLabel, 8, -1,
                 &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
    func_8006E2C4((s16)(x + 56), (s16)(y + 4), 1, 0, value + 1, 8, -1,
                 &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
    func_8006EF30((s16)(x + 8), (s16)(y + 20), 10, gTaskPlayerValuePanelText, 8, -1,
                 &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
    drawDefaultTexturedPanel((s16)x, (s16)y, 144, 48, 5, 0,
                            &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
}

INCLUDE_ASM("asm/nonmatchings/main_tail", func_800995A8);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_800996F8);

void initializeCountedFourBoundsTask(void) {
    D_8013AE98->data1C++;
    D_8013AE88->data18 = 0;
    D_8013AE88->data1A = 0;
    if (D_8013AE88->resource14 != 0xFFFF) { for (;;) {} }
    D_8013AE88->resource14 = allocateTaskResource();
    if (D_8013AE88->resource16 != 0xFFFF) { for (;;) {} }
    D_8013AE88->resource16 = allocateTaskResource();
    setSecondaryDisplayTaskMode(2);
    {
        RECT *bounds = &D_801300B8[0];
        bounds->x = 0;
        bounds->y = 0;
        bounds->w = -1;
        bounds->h = -1;
    }
    {
        RECT *bounds = &D_801300B8[1];
        bounds->x = 0;
        bounds->y = 0;
        bounds->w = -1;
        bounds->h = -1;
    }
    {
        RECT *bounds = &D_801300B8[2];
        bounds->x = 24;
        bounds->y = 132;
        bounds->w = 120;
        bounds->h = 84;
    }
    {
        RECT *bounds = &D_801300B8[3];
        bounds->x = 0;
        bounds->y = 0;
        bounds->w = -1;
        bounds->h = -1;
    }
    D_8013B517 = 1;
    setCurrentTaskCallback(func_80099880);
    D_8013AE88->unk10 |= 2;
}

INCLUDE_ASM("asm/nonmatchings/main_tail", func_80099880);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_8009A560);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_8009A87C);

void initializeTaskGridPositions(s32 unused, GameTask *task, s32 x, s32 y) {
    s32 i;
    TaskGridState *state;
    TaskGridEntry *entry;
    state = (TaskGridState *)&D_800D9504[task->resource14];
    entry = (TaskGridEntry *)&D_800D9504[task->resource16];
    for (i = 0; i < state->count; i++) {
        if (i < 8) {
            initMovement2D((Movement2D *)&D_800D9504[entry[i].task->resource14],
                           x + (i % 4) * 0x20000, y + (i / 4) * 0x28000);
        } else {
            initMovement2D((Movement2D *)&D_800D9504[entry[i].task->resource14],
                           x + 0xC000, y + 0x50000);
        }
    }
}

void configureFourColumnTaskGridMovement(s32 mode, GameTask *task, s32 x, s32 y, s32 stepX, s32 stepY, s32 delay) {
    TaskGridState *state;
    TaskGridEntry *entries;
    s16 i, end;
    Movement2D *movement;
    state = (TaskGridState *)&D_800D9504[task->resource14];
    entries = (TaskGridEntry *)&D_800D9504[task->resource16];
    if (mode == 1) {
        i = state->selectedIndex;
        end = i + 1;
    } else {
        i = 0;
        end = state->count;
    }
    for (; i < end; i++) {
        if (mode == 2 && i == state->selectedIndex) continue;
        if (i < 8) {
            movement = (Movement2D *)&D_800D9504[entries[i].task->resource14];
            if (delay == -1000) {
                configureMovement2D(movement, x + (i % 4) * 0x20000,
                                    y + (i / 4) * 0x28000, stepX, stepY, delay);
            } else {
                configureMovement2D(movement, x + (i % 4) * 0x20000,
                                    y + (i / 4) * 0x28000, stepX, stepY, delay * (i / 4));
            }
        } else {
            movement = (Movement2D *)&D_800D9504[entries[i].task->resource14];
            if (delay == -1000) {
                configureMovement2D(movement, x + 0xC000, y + 0x50000, stepX, stepY, delay);
            } else {
                configureMovement2D(movement, x + 0xC000, y + 0x50000, stepX, stepY, delay * 2);
            }
        }
    }
}

void drawTaskGridSelectionMarker(void) {
    GridMarkerState *state;
    TaskGridEntry *entries;
    Movement2D *position;
    s32 mode;
    u32 colorMode;
    if (D_8013AE88->resource14 == 0xFFFF) { for (;;) {} }
    state = (GridMarkerState *)&D_800D9504[D_8013AE88->resource14];
    if (D_8013AE88->resource16 == 0xFFFF) { for (;;) {} }
    entries = (TaskGridEntry *)&D_800D9504[D_8013AE88->resource16];
    mode = (s16)state->state;
    colorMode = state->state;
    if (mode > 0) {
        if ((u32)(colorMode - 1) < 2) {
            setSpriteColor(state->red, state->green, state->blue);
        } else {
            setSpriteColor(128, 128, 128);
        }
        position = (Movement2D *)&D_800D9504[entries[state->selectedIndex].task->resource14];
        if (state->selectedIndex < 8) {
            func_8006C998(position->x / 4096, position->y / 4096, 0x16F, 8, -1,
                         &D_800EBFF0[D_8013AF78], (u16)(state->depth - 1));
        } else {
            func_8006C998(position->x / 4096, position->y / 4096, 0xB0, 0, -1,
                         &D_800EBFF0[D_8013AF78], (u16)(state->depth - 1));
        }
    }
}

INCLUDE_ASM("asm/nonmatchings/main_tail", func_8009B2D8);

void copyPlayerPaletteRowWithTransition(s16 playerIndex, s32 unused, s32 paletteRow, u16 mode) {
    RECT rect;
    PlayerTextureOrigin *origin = &D_801204D4[playerIndex];
    if (mode == 0) {
        rect.x = origin->x + 0x210;
        rect.y = origin->y + (s16)(paletteRow + 0x70);
        rect.w = 16;
        rect.h = 1;
        MoveImage(&rect, origin->x + 0x200, origin->y + 0x70);
    } else {
        mode++;
        rect.x = origin->x + 0x210;
        rect.y = origin->y + (s16)(paletteRow + 0x70);
        rect.w = 16;
        rect.h = 1;
        StoreImage(&rect, gPlayerPaletteTransitionPixels);
        startPaletteTransition(origin->x + 0x200, origin->y + 0x70, gPlayerPaletteTransitionPixels, mode);
    }
}

void copyPlayerPaletteRow(s16 playerIndex) {
    copyPlayerPaletteRowWithTransition(playerIndex, D_80120E90[playerIndex].unk009,
                  D_80120E90[playerIndex].unk00D, 0);
}

INCLUDE_ASM("asm/nonmatchings/main_tail", func_8009B700);

void spawnPaletteBurstTask(u8 paletteIndex) {
    u16 pixels[16];
    RECT rect;
    PaletteBurstResource *resource;
    u8 row;
    func_80014930(&gGameTasks[4], func_8009B700);
    row = paletteIndex;
    D_8013AE8C->resource14 = allocateTaskResource();
    resource = (PaletteBurstResource *)&D_800D9504[D_8013AE8C->resource14];
    resource->x = randomNextLinear() % 4 - 2;
    resource->y = randomNextLinear() % 4 - 17;
    resource->z = randomNextLinear() % 4 - 2;
    resource->rotation = randomNextLinear() % 4096;
    resource->age = 0;
    if (row >= 8) row = paletteIndex + 1;
    rect.x = 1008;
    rect.y = row + 240;
    rect.w = 16;
    rect.h = 1;
    StoreImage(&rect, (u32 *)pixels);
    DrawSync(0);
    resource->b = (pixels[1] >> 7) & 0xF8;
    resource->g = (pixels[1] >> 2) & 0xF8;
    resource->r = pixels[1] << 3;
}

void initializeMainTailDualResourceTask(void) {
    GameTask *task = D_8013AE88;
    GameTask *parent = D_8013AE98;
    task->data18 = 0;
    parent->data1C++;
    if (task->resource14 != 0xFFFF) {
        while (1) {}
    }
    D_8013AE88->resource14 = allocateTaskResource();
    if (D_8013AE88->resource16 != 0xFFFF) {
        while (1) {}
    }
    D_8013AE88->resource16 = allocateTaskResource();
    setCurrentTaskCallback(func_8009BDB8);
    D_8013AE88->unk10 |= 2;
}

INCLUDE_ASM("asm/nonmatchings/main_tail", func_8009BDB8);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_8009C568);

void initializeSelectionIndexedDualResourceTask(void) {
    GameTask *task = D_8013AE88;
    task->data18 = 0;
    if (D_8013B527 != 0) {
        task->data1A = D_8013B527 - 1;
    } else {
        task->data1A = D_8013B527;
    }
    D_8013AE98->data1C++;
    if (D_8013AE88->resource14 != 0xFFFF) {
        for (;;) {}
    }
    D_8013AE88->resource14 = allocateTaskResource();
    if (D_8013AE88->resource16 != 0xFFFF) {
        for (;;) {}
    }
    D_8013AE88->resource16 = allocateTaskResource();
    setCurrentTaskCallback(func_8009CA90);
    D_8013AE88->unk10 |= 2;
}

INCLUDE_ASM("asm/nonmatchings/main_tail", func_8009CA90);

void initializeModeDependentTaskResourcePair(void) {
    D_8013AE88->data18 = 0;
    if (D_8013AFCD == 1) {
        D_8013AE88->data1A = 1;
    } else {
        D_8013AE88->data1A = 3;
    }
    if (D_8013AE88->resource14 != 0xFFFF) {
        for (;;) {}
    }
    D_8013AE88->resource14 = allocateTaskResource();
    if (D_8013AE88->resource16 != 0xFFFF) {
        for (;;) {}
    }
    D_8013AE88->resource16 = allocateTaskResource();
    setCurrentTaskCallback(func_8009D354);
}

INCLUDE_ASM("asm/nonmatchings/main_tail", func_8009D354);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_8009DB70);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_8009DEF0);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_8009E468);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_8009E848);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_8009ECCC);

void initializeFlagSelectedDualResourceTask(void) {
    GameTask *task;
    GameTask *parent;
    D_8013AE88->data18 = 0;
    D_8013AE88->data1A = func_8007D47C();
    task = D_8013AE88;
    parent = D_8013AE98;
    parent->data1C++;
    if (task->resource14 != 0xFFFF) {
        while (1) {}
    }
    D_8013AE88->resource14 = allocateTaskResource();
    if (D_8013AE88->resource16 != 0xFFFF) {
        while (1) {}
    }
    D_8013AE88->resource16 = allocateTaskResource();
    setCurrentTaskCallback(func_8009F224);
    D_8013AE88->unk10 |= 2;
}

INCLUDE_ASM("asm/nonmatchings/main_tail", func_8009F224);

void initializeModeDependentTailResourcePairTask(void) {
    GameTask *task = D_8013AE88;
    task->data18 = 0;
    if (D_8013AFCD == 1) {
        task->data1A = 1;
    } else {
        task->data1A = 3;
    }
    if (D_8013AE88->resource14 != 0xFFFF) {
        while (1) {}
    }
    D_8013AE88->resource14 = allocateTaskResource();
    if (D_8013AE88->resource16 != 0xFFFF) {
        while (1) {}
    }
    D_8013AE88->resource16 = allocateTaskResource();
    setCurrentTaskCallback(func_8009F9B8);
}

INCLUDE_ASM("asm/nonmatchings/main_tail", func_8009F9B8);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_800A008C);

s16 findIdentityMappedIndexOrZero(ByteMappingDescriptor *descriptor, s16 value) {
    s16 result = 0;
    s16 i;

    for (i = 0; i < descriptor->count; i++) {
        if (value == (s16)func_8007373C(descriptor->mapping, value)) {
            result = i;
            break;
        }
    }
    return result;
}

void initializeTaskResourcePairWithOneTickTransition(void) {
    D_8013AE88->data18 = 0;
    if (D_8013AE88->resource14 != 0xFFFF) {
        for (;;) {}
    }
    D_8013AE88->resource14 = allocateTaskResource();
    if (D_8013AE88->resource16 != 0xFFFF) {
        for (;;) {}
    }
    D_8013AE88->resource16 = allocateTaskResource();
    configureTaskStateTransition(2, 0, 1);
    setCurrentTaskCallback(func_800A05C0);
    D_8013AE88->unk10 |= 2;
}

INCLUDE_ASM("asm/nonmatchings/main_tail", func_800A05C0);

void selectTaskCallbackFromModeState(void) {
    if (D_8013AFD8 != 0) {
        setCurrentTaskCallback(initializeMode1RacePlayers);
    } else if (D_8013AFD9 != 0) {
        setCurrentTaskCallback(initializeControllerPlaybackRaceTask);
    }
}

void initializeMode1Task(void) {
    D_8013AFD8 = 1;
    D_8013AFCE = -1;
    D_8013B560 = 99;
    setControllerFlag5(0);
    setControllerFlag5(1);
    D_8013B0F0 = 0;
    setCurrentTaskCallback(initializeMode1RacePlayers);
}

void initializeMode1RacePlayers(void) {
    s32 i;
    s16 mode;
    D_8013B560 = D_8013AFCE;
    if ((s16)D_8013AFD8 < 7 && D_8013AFD8 != 0) D_8013AFCE = 0;
    else D_8013AFCE = 9;
    SetFogNearFar(8000, 10000, 400);
    D_8013AFD3 = 0;
    D_8013AFD6 = 0;
    D_8013AFE1 = 0;
    D_8013AFD9 = 0;
    D_8013AFE2 = 0;
    D_8013AFE0 = 0;
    D_80120E90[0].unk008 = 0;
    D_80120E90[1].unk008 = 1;
    D_80120E90[2].unk008 = 1;
    D_80120E90[3].unk008 = 1;
    D_80120E90[0].unk014 = 0;
    D_80120E90[1].unk014 = 1;
    D_80120E90[2].unk014 = 2;
    D_80120E90[3].unk014 = 3;
    D_80120E90[0].unk015[1] = 0;
    D_80120E90[1].unk015[1] = 0;
    D_80120E90[2].unk015[1] = 0;
    D_80120E90[3].unk015[1] = 0;
    D_80120E90[0].unk015[2] = 3;
    D_80120E90[1].unk015[2] = 5;
    D_80120E90[2].unk015[2] = 6;
    D_80120E90[3].unk015[2] = 7;
    D_80120E90[0].unk025[3] = 0;
    D_80120E90[1].unk025[3] = 1;
    D_80120E90[2].unk025[3] = 2;
    D_80120E90[3].unk025[3] = 3;
    gameOptionPlayerModeAbsolute = 2;
    mode = D_8013AFD8;
    if (mode != 0 && (mode < 7 || mode == 9)) {
        D_80120E90[0].unk024 = 1;
        D_80120E90[1].unk024 = 0;
        D_80120E90[2].unk024 = 0;
        D_80120E90[3].unk024 = 0;
        D_8013AFD2 = 1;
    } else {
        D_80120E90[0].unk024 = 1;
        D_80120E90[1].unk024 = 1;
        D_80120E90[2].unk024 = 1;
        D_80120E90[3].unk024 = 1;
        D_8013AFD2 = 4;
    }
    D_8013AFD0 = 5;
    D_8013AFDE = 100;
    for (i = 0; i < 4; i++) {
        D_80120E90[i].unk025[1] = 0;
        D_80120E90[i].unk025[2] = 0;
        D_80120E90[i].unk01C = 0;
    }
    setCurrentTaskCallback(prepareRaceResourcesIfSelectionChanged);
}

void prepareRaceResourcesIfSelectionChanged(void) {
    configureTaskStateTransition(1, 0, 30);
    setSecondaryDisplayTaskMode(1);
    if (D_8013B560 != D_8013AFCE) {
        func_800180F4(2);
        func_800180F4(3);
        func_800180F4(4);
        func_800180F4(5);
        queueStartupSectorReads();
        setCurrentTaskCallback(preparePlayerResourcesTask);
        D_8013AE88->unk10 |= 2;
    } else {
        setCurrentTaskCallback(initializeRaceTaskState);
    }
}

void preparePlayerResourcesTask(void) {
    queuePlayerResourceReadsAndUploadTextures();
    setCurrentTaskCallback(initializeRaceTaskState);
    D_8013AE88->unk10 |= 2;
}

void initializeModeSectorReadResourcesTask(void) {
    ModeSectorReadState *state;
    if (D_8013AE88->resource14 != 0xFFFF) {
        for (;;) {}
    }
    D_8013AE88->resource14 = allocateTaskResource();
    state = (ModeSectorReadState *)&D_800D9504[D_8013AE88->resource14];
    if (D_8013AE88->resource16 != 0xFFFF) {
        for (;;) {}
    }
    D_8013AE88->resource16 = allocateTaskResource();
    D_8013B55E = 0;
    if (D_8013AFD8 != 0) {
        state->mode = 1;
    } else {
        state->mode = 2;
    }
    stopPlayerSound(0);
    stopPlayerSound(1);
    setCurrentTaskCallback(queueTaskModeSectorRead);
}

void queueTaskModeSectorRead(void) {
    ModeSectorReadState *state;
    if (D_8013AE88->resource14 == 0xFFFF) {
        for (;;) {}
    }
    state = (ModeSectorReadState *)&D_800D9504[D_8013AE88->resource14];
    if (state->mode == 1) {
        state->sectorIndex = D_8013AFD8 - 1;
    } else {
        state->sectorIndex = D_8013AFD9 - 1;
    }
    queueModeIndexedSectorRead(state->mode, state->sectorIndex);
    setCurrentTaskCallback(func_800A0EE8);
    D_8013AE88->unk10 |= 2;
}

INCLUDE_ASM("asm/nonmatchings/main_tail", func_800A0EE8);

void initializeTextSequenceTask(void) {
    TextSequenceResource *resource;
    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (TextSequenceResource *)&D_800D9504[D_8013AE88->resource14];
    resource->state = 2;
    resource->entryIndex = 0;
    resource->progress = 0;
    resource->duration = 0;
    resource->intensity = 0x80;
    resource->unk3A = 0;
    setSecondaryDisplayTaskMode(0);
    configureTaskStateTransition(1, 0, 30);
    if (resource->mode != 1 || resource->unk0A != 9) {
        queueCdCommand(3, 0);
    }
    setCurrentTaskCallback(func_800A0FDC);
}

INCLUDE_ASM("asm/nonmatchings/main_tail", func_800A0FDC);

void animateTextSequenceExitTask(void) {
    TextSequenceResource *resource;
    u16 previous;
    s16 delta;
    if (D_8013AE88->resource14 == 0xFFFF) {
        for (;;) {}
    }
    resource = (TextSequenceResource *)&D_800D9504[D_8013AE88->resource14];
    resource->velocity += 0x1000;
    resource->y -= resource->velocity;
    resource->displacement += resource->velocity;
    if (resource->intensity != 0) resource->intensity -= 4;
    previous = D_800EC158[1].h;
    D_800EC158[1].h = resource->displacement / 4096 - (u32)(-D_8013AF7C / 2 + 4);
    if (D_800EC158[1].h > D_8013AF7C) D_800EC158[1].h = D_8013AF7C;
    delta = D_800EC158[1].h - previous;
    resource->unk3A += delta;
    D_8013AF80 -= delta / 2;
    if (D_8013AF80 < 0) D_8013AF80 = 0;
    D_800EC158[2].y += delta;
    D_800EC158[2].h -= delta;
    if (D_800EC158[2].h < 0) {
        D_800EC158[2].y = 240;
        D_800EC158[2].h = 0;
    }
    D_801241E8[0].unk10 += delta * 2;
    if (D_801241E8[0].unk10 > -320) D_801241E8[0].unk10 = -320;
    if (resource->displacement / 4096 >= 141) {
        taskTransitionFlagGp = 1;
        D_8013B1A8 = 1;
        D_8013AFCD = 1;
        D_8013AFE4 = 0;
        D_8013AFAA |= 0x1000;
        queueCdCommand(D_80124164[D_8013AFCE], 0);
        func_800149A8();
    } else {
        func_800A1774(resource);
    }
}

void advanceModeAfterTextSequenceFadeTask(void) {
    TextSequenceResource *resource;
    if (D_8013AE88->resource14 == 0xFFFF) {
        for (;;) {}
    }
    resource = (TextSequenceResource *)&D_800D9504[D_8013AE88->resource14];
    switch (D_8013AE88->data18) {
    case 1:
        configureTaskStateTransition(2, 0, 30);
        startCdVolumeFade(103, 20, 0, 1);
        resource->intensity = 0;
        D_8013AE88->data18 = 2;
        break;
    case 2:
        if (func_800707F0(0) == 0) {
            D_8013B55E = 2;
            D_8013AFD8++;
            func_800149A8();
        }
        break;
    }
    func_800A1774(resource);
}

void queueModeIndexedSectorRead(s16 mode, s16 index) {
    if (mode == 1) {
        queueCdSectorCommand(0, gMode1SectorRanges[index].sector,
                      gMode1SectorRanges[index].sectorCount,
                      INDEXED_SECTOR_READ_DESTINATION, 0);
    } else {
        queueCdSectorCommand(0, gOtherModeSectorRanges[index].sector,
                      gOtherModeSectorRanges[index].sectorCount,
                      INDEXED_SECTOR_READ_DESTINATION, 0);
    }
}

INCLUDE_ASM("asm/nonmatchings/main_tail", func_800A1670);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_800A1774);

void initializeTaskPlayerCamera(void) {
    PlayerCamera *cameras;
    D_8013B0D8 = &D_80120E90[D_8013AE88->data18];
    initPlayerCameraAndTarget(D_8013AE88->data18);
    cameras = D_801241E8;
    cameras[D_8013AE88->data18].yaw = D_8013B0D8->unk110;
    cameras[D_8013AE88->data18].pitch = 0;
    cameras[D_8013AE88->data18].unk18 = D_8013B0D8->unk92E;
    updatePlayerCameraPosition(D_8013AE88->data18);
    clampPlayerCameraHeight(D_8013AE88->data18);
    cameras[D_8013AE88->data18].unk10 = -0x230;
    setCurrentTaskCallback(updateTaskViewAndCheckPlayerLimit);
}

void updateTaskViewAndCheckPlayerLimit(void) {
    func_8003D940();
    if (D_8013B55E != 0) {
        if (D_8013AE88->data18 < D_8013AFCD) {
            setCurrentTaskCallback(func_8003EE70);
            return;
        }
        func_800149A8();
    }
    updatePlayerCameraPosition(D_8013AE88->data18);
}
