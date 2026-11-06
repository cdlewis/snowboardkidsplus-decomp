#include "common.h"
#include "main_tail_sound_slots.h"
#include "game/controller.h"
#include "game/global_flags.h"
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
#include "game/text_entries.h"
#include "game/task.h"
#include "game/movement.h"
#include "psx/libgpu.h"
#include "psx/libgs.h"

#define INDEXED_SECTOR_READ_DESTINATION ((void *)0x801F2080)

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
extern u8 D_8013B55E __attribute__((section(".data")));

typedef struct TextEntryListHeader {
    TextEntry *freeEntries;
    TextEntry *activeEntries;
} TextEntryListHeader;

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

s32 queueCdSectorCommand(s16 command, s16 sector, s16 sectorCount, void *destination, s32 flags);

void func_800187B8(u8 voice);

void func_80090450(void);
void func_800A0AA8(void);
void func_800A0EE8(void);
void queueModeIndexedSectorRead(s16 mode, s16 index);
void func_800A3750(void);
void func_800A5AFC(void);
void func_800A6548(void);

void func_8002FA58(s16 resource);
void func_800A39AC(void);
void func_800A42CC(void);
void func_800A499C(void);
void func_8001D97C(void);
void initializeRaceTaskState(void);
void func_8001B858(void);
void func_8001B7F0(void);
void func_800A1F50(void);
void func_8009B5A0(s16 playerIndex, s32 unused, s32 paletteRow, u16 mode);
void func_800A4C28(void);
void resetUnlockMenuOffsets(UnlockMenuState *state);
void func_800A525C(void);
void func_800A6A8C(void);
void func_800A624C(void);
void startUnlockResourceLoad(void);
void prepareUnlockSelectionResume(void);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_8008C708);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_8008C8BC);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_8008CB88);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_8008CD98);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_8008D144);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_8008D200);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_8008D958);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_8008DD04);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_8008DDC0);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_8008E66C);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_8008EB94);

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

INCLUDE_ASM("asm/nonmatchings/main_tail", func_80091FAC);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_80092090);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_800929C4);

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

INCLUDE_ASM("asm/nonmatchings/main_tail", func_800964B0);

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

INCLUDE_ASM("asm/nonmatchings/main_tail", func_80098F38);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_80099010);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_8009919C);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_80099318);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_800995A8);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_800996F8);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_8009974C);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_80099880);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_8009A560);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_8009A87C);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_8009ADAC);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_8009AEC0);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_8009B0E8);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_8009B2D8);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_8009B5A0);

void copyPlayerPaletteRow(s16 playerIndex) {
    func_8009B5A0(playerIndex, D_80120E90[playerIndex].unk009,
                  D_80120E90[playerIndex].unk00D, 0);
}

INCLUDE_ASM("asm/nonmatchings/main_tail", func_8009B700);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_8009BB68);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_8009BCFC);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_8009BDB8);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_8009C568);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_8009C9A0);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_8009CA90);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_8009D29C);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_8009D354);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_8009DB70);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_8009DEF0);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_8009E468);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_8009E848);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_8009ECCC);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_8009F158);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_8009F224);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_8009F900);

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

INCLUDE_ASM("asm/nonmatchings/main_tail", func_800A050C);

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

INCLUDE_ASM("asm/nonmatchings/main_tail", func_800A0F14);

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

INCLUDE_ASM("asm/nonmatchings/main_tail", func_800A1DA0);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_800A1ECC);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_800A1F50);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_800A2210);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_800A2324);

void selectTaskBySelectedRecordMatch(void) {
    if (D_8013B560 != D_8013AFCE) {
        D_8013B562 = 0;
        setCurrentTaskCallback(func_8001B858);
    } else {
        D_8013B562 = 1;
        setCurrentTaskCallback(func_800A1F50);
    }
}

void startMode1TaskTransition(void) {
    func_800719A8(1);
    func_80070274(1, 0, 30);
    setCurrentTaskCallback(func_8001B7F0);
}

INCLUDE_ASM("asm/nonmatchings/main_tail", func_800A24EC);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_800A2510);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_800A25F8);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_800A29A4);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_800A2AB0);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_800A2C40);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_800A2C74);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_800A2D68);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_800A3028);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_800A31C0);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_800A3750);

void advanceTaskWhenFlagB55ESet(void) {
    if (D_8013B55E != 0) {
        D_8013AE88->data18 = 0;
        setCurrentTaskCallback(func_800A39AC);
    }
}

INCLUDE_ASM("asm/nonmatchings/main_tail", func_800A39AC);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_800A3AA4);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_800A3C3C);

void resetUnlockMenuAnimation(UnlockMenuState *state) {
    s16 i;
    state->unk04[0] = state->unk20[0] = 0;
    for (i = 1; i < 7; i++) {
        state->unk04[i] = state->unk3C[i] = state->unk04[0];
        state->unk20[i] = state->unk58[i] = state->unk20[0];
    }
    state->unk3C[0] = state->unk04[0];
    state->unk58[0] = state->unk20[0];
    state->unk74[0] = 0;
    state->unk74[1] = 1;
    state->unk74[2] = 2;
    state->unk74[3] = 3;
}

void runTaskResource14Update(void) {
    if (D_8013AE88->resource14 == 0xFFFF) {
        for (;;) {
        }
    }
    func_800A42CC();
}

INCLUDE_ASM("asm/nonmatchings/main_tail", func_800A3D9C);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_800A42CC);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_800A46E0);

void initializeUnlockMenuTask(void) {
    UnlockMenuState *state;
    if (D_8013AE88->resource14 == 0xFFFF) {
        for (;;) {}
    }
    state = (UnlockMenuState *)&D_800D9504[D_8013AE88->resource14];
    state->selection = 0;
    state->previousSelection = 0;
    resetUnlockMenuOffsets(state);
    func_800A4C28();
    setCurrentTaskCallback(func_800A499C);
}

void resetUnlockMenuOffsets(UnlockMenuState *state) {
    s16 i;
    for (i = 0; i < 6; i++) {
        i[state->unk3C] = 0;
        i[state->unk04] = 0;
        i[state->unk58] = 0;
        i[state->unk20] = 0;
    }
}

INCLUDE_ASM("asm/nonmatchings/main_tail", func_800A499C);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_800A4C28);

s32 hasUnlockNotificationFlag(s16 index) {
    if (index == 11) {
        index = 10;
    }
    return (s16)getUnlockNotificationFlag(index) != 0;
}

void startUnlockActionAfterCountdown(void) {
    if (--D_8013AE88->data18 == 0) {
        if (D_8013AF3A < 11) {
            func_80014874(func_800A624C);
        } else {
            func_80014874(startUnlockResourceLoad);
        }
        setCurrentTaskCallback(prepareUnlockSelectionResume);
        D_8013AE88->unk10 |= 1;
    }
}

INCLUDE_ASM("asm/nonmatchings/main_tail", func_800A5050);

void resumeUnlockSelectionTask(void) {
    func_800719A8(0);
    setCurrentTaskCallback(func_800A499C);
}

void prepareUnlockSelectionResume(void) {
    func_800719A8(3);
    D_8013B10C = -1;
    func_8002FA58(2);
    setCurrentTaskCallback(resumeUnlockSelectionTask);
    D_8013AE88->unk10 |= 2;
}

INCLUDE_ASM("asm/nonmatchings/main_tail", func_800A525C);

void startUnlockResourceLoad(void) {
    resetPendingCdRequests();
    D_8013AFE4 &= ~0x100;
    createResourceLoadTask();
    setCurrentTaskCallback(func_800A525C);
}

INCLUDE_ASM("asm/nonmatchings/main_tail", func_800A52D8);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_800A53BC);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_800A5418);

void drawNumberAdjustmentArrow(s16 direction, GsOT *ot, u16 depth) {
    POLY_F3 *triangle = &gNumberAdjustmentArrow;
    setRGB0(triangle, 0xC0, 0x20, 0);
    if (direction == 0) {
        setXY3(triangle, -46, -32, -40, -44, -34, -32);
    } else {
        setXY3(triangle, -46, -8, -40, 4, -34, -8);
    }
    GsSortPoly(triangle, ot, depth);
}

INCLUDE_ASM("asm/nonmatchings/main_tail", func_800A5630);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_800A59B8);

void func_800A5AD4(void) {
}

void func_800A5ADC(void) {
}

void func_800A5AE4(void) {
}

void func_800A5AEC(void) {
}

void func_800A5AF4(void) {
}

INCLUDE_ASM("asm/nonmatchings/main_tail", func_800A5AFC);

s16 parseDecimalS16(const u8 *text, s16 *result) {
    s16 value = 0;
    s16 count = 0;
    u16 digit;
    while ((u32)(*text - '0') < 10) {
        count++;
        digit = *text++;
        value = value * 10 + (digit - (u16)'0');
    }
    *result = value;
    return count;
}

void resetTextEntryStates(TextControlState *control, TextResourceState *resource) {
    s16 i;
    if (control != NULL) {
        control->unk00 = 0;
        control->unk02 = 1;
        control->unk04 = 0;
        control->unk08 = 0;
        control->unk0C = 0;
        for (i = 0; i < 5; i++) {
            control->unk18[i] = 0;
        }
    }
    if (resource != NULL) {
        for (i = 0; i < 5; i++) {
            resource->unk04[i] = 0;
            resource->unk18[i] = 0;
        }
    }
}

void resetTextEntryPool(void) {
    TextEntry *previous = NULL;
    TextEntry *entry = (TextEntry *)0x80180008;
    u16 i;
    D_8013B5D0 = entry;
    for (i = 0; i < 295; i++) {
        entry->previous = previous;
        previous = entry;
        previous->unk04 = 0;
        previous->unk06 = 0;
        previous->unk08 = 0;
        previous->unk0A = 0;
        previous->next = previous + 1;
        entry++;
    }
    entry[-1].next = NULL;
    D_8013B5D4 = NULL;
}

INCLUDE_ASM("asm/nonmatchings/main_tail", func_800A5F00);

TextEntry *findKeyedListNode(u32 key, u32 value) {
    TextEntry *node = D_8013B5D4;
    TextEntry *result = NULL;
    while (node != NULL) {
        if (key == (u32)node->text && value == node->unk04) {
            result = node;
            break;
        }
        node = node->next;
        result = NULL;
    }
    return result;
}

TextEntry *findTextEntryInsertionPoint(u32 key, u32 value) {
    TextEntry *node = D_8013B5D4;
    while (node != NULL) {
        if (key == (u32)node->text && value < node->unk04) {
            break;
        }
        if (key < (u32)node->text) {
            break;
        }
        node = node->next;
    }
    return node;
}

INCLUDE_ASM("asm/nonmatchings/main_tail", func_800A5FE8);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_800A60FC);

void playMusicSequence(s16 count, u8 left, u8 right) {
    if (gMusicSequenceActive) {
        D_8013AF30 = left << 8;
        SsSeqSetVol(gMusicSequenceId, left, right);
        SsSeqPlay(gMusicSequenceId, 1, count);
    }
}

INCLUDE_ASM("asm/nonmatchings/main_tail", func_800A624C);

void initializeLoadedTextEntries(void) {
    TextEntryListHeader *header = (TextEntryListHeader *)0x80180000;
    TextEntry *freeEntries = header->freeEntries;
    TextEntry *activeEntries = header->activeEntries;
    D_8013B5D0 = freeEntries;
    D_8013B5D4 = activeEntries;
    func_800A5AFC();
    setCurrentTaskCallback(func_800A6548);
}

INCLUDE_ASM("asm/nonmatchings/main_tail", func_800A6548);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_800A6684);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_800A68B4);

void initializeSecondaryResourceTask(void) {
    if (D_8013AE98->resource16 == 0xFFFF) {
        while (1) {
        }
    }
    D_8013AE88->data18 = 0;
    setCurrentTaskCallback(func_800A6A8C);
    D_8013AE88->unk10 |= 2;
}

INCLUDE_ASM("asm/nonmatchings/main_tail", func_800A6A8C);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_800A6CC4);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_800A6DD0);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_800A6E88);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_800A73BC);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_800A7590);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_800A7664);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_800A77B0);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_800A7928);

void stopTextSoundVoices(void) {
    s16 i;
    for (i = 0; i < 8; i++) {
        if (gTextSoundSlots[i].voice != 0xFF) {
            func_800187B8(gTextSoundSlots[i].voice);
            gTextSoundSlots[i].voice = 0xFF;
            gTextSoundSlots[i].soundKey = 0;
        }
    }
}

INCLUDE_ASM("asm/nonmatchings/main_tail", func_800A7B9C);

void signalFlag9WhenTaskCountdownExpires(void) {
    if (--D_8013AE88->data18 == 0) {
        D_8013AFE4 |= 0x200;
        func_800149A8();
    }
}

INCLUDE_ASM("asm/nonmatchings/main_tail", func_800A7CE8);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_800A7D8C);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_800A7F84);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_800A8044);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_800A8104);
