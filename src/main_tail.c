#include "common.h"
#include "main_tail_sound_slots.h"
#include "game/controller.h"
#include "game/global_flags.h"
#define SELECTED_RECORDS_ABSOLUTE_ADDRESSING
#include "game/selected_records.h"
#include "game/cd_queue.h"
#include "game/byte_search.h"
#include "game/game_options.h"
#include "game/graphics.h"
#define MUSIC_ABSOLUTE_ADDRESSING
#include "game/music.h"
#include "psx/libsnd.h"
#define RACE_PLAYER_ABSOLUTE_ADDRESSING
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

#define INDEXED_SECTOR_READ_DESTINATION ((void *)0x801F2080)

/* Menu descriptors have a 0x14-byte stride in the callers' tables. */
typedef struct ByteMappingDescriptor {
    const u8 *mapping;
    u8 unk04[2];
    u8 count;
    u8 unk07[13];
} ByteMappingDescriptor;

typedef struct TaskGridState {
    u8 unk00[0x5A];
    s16 count;
} TaskGridState;

typedef struct TaskGridEntry {
    GameTask *task;
    u32 unk04;
} TaskGridEntry;

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
    u8 unk0E[0x22];
    u16 progress;
    u16 duration;
    s16 state;
    u8 intensity;
    u8 unk37;
    s16 mode;
    u16 unk3A;
    u8 unk3C[0xC4];
} TextSequenceResource;

typedef struct ResourceSectorRange {
    s16 sector;
    s16 sectorCount;
} ResourceSectorRange;

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

extern s16 D_8013B560;
extern u8 D_8013B562;
extern u8 D_8013B0F0 __attribute__((section(".data")));
extern u8 D_8013AF3A __attribute__((section(".data")));


void func_800187B8(u8 voice);

void func_8008D200(void);
void func_8008DDC0(void);
void func_8008EC50(void);
void func_80090450(void);
void func_80092090(void);
void func_800A0FDC(void);
void func_80092A50(void);
void func_800A0AA8(void);
void func_800A0EE8(void);
void queueModeIndexedSectorRead(s16 mode, s16 index);
void func_800A3750(void);
void func_800A5AFC(void);
void func_800A6548(void);
void func_8009BDB8(void);
void func_8009CA90(void);
void func_8009F9B8(void);
void func_8009F224(void);
s32 func_8007D47C(void);

void func_8002FA58(s16 resource);
void func_800A39AC(void);
void func_800A42CC(void);
void func_800A499C(void);
void func_8001D97C(void);
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
void func_8003E050(u16 index);
void updateTaskViewAndCheckPlayerLimit(void);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_8008C708);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_8008C8BC);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_8008CB88);

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

INCLUDE_ASM("asm/nonmatchings/main_tail", func_8008F970);

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
    D_801300AC.selection = 0;
    D_801300AC.previousSelection = 0;
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

INCLUDE_ASM("asm/nonmatchings/main_tail", func_80094564);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_80094760);

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

INCLUDE_ASM("asm/nonmatchings/main_tail", func_80095F30);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_800960BC);

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

INCLUDE_ASM("asm/nonmatchings/main_tail", func_800972B8);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_8009747C);

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

INCLUDE_ASM("asm/nonmatchings/main_tail", func_8009766C);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_80097858);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_80097C2C);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_80097EBC);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_80097FF4);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_80098D68);

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

INCLUDE_ASM("asm/nonmatchings/main_tail", func_80099010);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_8009919C);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_80099318);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_800995A8);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_800996F8);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_8009974C);

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

INCLUDE_ASM("asm/nonmatchings/main_tail", func_8009AEC0);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_8009B0E8);

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

INCLUDE_ASM("asm/nonmatchings/main_tail", func_8009BB68);

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
    func_80070274(2, 0, 1);
    setCurrentTaskCallback(func_800A05C0);
    D_8013AE88->unk10 |= 2;
}

INCLUDE_ASM("asm/nonmatchings/main_tail", func_800A05C0);

void selectTaskCallbackFromModeState(void) {
    if (D_8013AFD8 != 0) {
        setCurrentTaskCallback(func_800A0AA8);
    } else if (D_8013AFD9 != 0) {
        setCurrentTaskCallback(func_800A3750);
    }
}

void initializeMode1Task(void) {
    D_8013AFD8 = 1;
    D_8013AFCE = -1;
    D_8013B560 = 99;
    setControllerFlag5(0);
    setControllerFlag5(1);
    D_8013B0F0 = 0;
    setCurrentTaskCallback(func_800A0AA8);
}

INCLUDE_ASM("asm/nonmatchings/main_tail", func_800A0AA8);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_800A0C6C);

void preparePlayerResourcesTask(void) {
    func_8001D97C();
    setCurrentTaskCallback(initializeRaceTaskState);
    D_8013AE88->unk10 |= 2;
}

INCLUDE_ASM("asm/nonmatchings/main_tail", func_800A0D54);

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
    func_80070274(1, 0, 30);
    if (resource->mode != 1 || resource->unk0A != 9) {
        queueCdCommand(3, 0);
    }
    setCurrentTaskCallback(func_800A0FDC);
}

INCLUDE_ASM("asm/nonmatchings/main_tail", func_800A0FDC);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_800A12E4);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_800A1510);

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
    func_8003E60C(D_8013AE88->data18);
    cameras = D_801241E8;
    cameras[D_8013AE88->data18].yaw = D_8013B0D8->unk110;
    cameras[D_8013AE88->data18].pitch = 0;
    cameras[D_8013AE88->data18].unk18 = D_8013B0D8->unk92E;
    func_8003E050(D_8013AE88->data18);
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
    func_8003E050(D_8013AE88->data18);
}
