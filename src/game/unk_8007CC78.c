#include "common.h"
#include "game/global_flags.h"
#include "game/graphics.h"
#include "game/game_options.h"
#include "game/linear_random.h"
#include "game/offset_archive.h"
#include "game/music.h"
#include "psx/libsnd.h"

typedef struct SaveRecord {
    u8 unk_000[0x3AC];
    u32 flags_3AC;
    u32 flags_3B0[10];
    u32 flags_3D8;
    u8 unk_3DC[0x2C];
} SaveRecord;

typedef struct HalfwordPair {
    s16 unk_00;
    s16 unk_02;
} HalfwordPair;

typedef struct ByteTriplet {
    u8 bytes[3];
} ByteTriplet;

typedef struct StateValueHistory {
    s32 unk_00;
    s16 activeIndex;
    s16 current[2];
    s16 previous[2];
    s16 unk_0E[2];
    s16 unk_12[2];
    s16 unk_16[2];
} StateValueHistory;

HalfwordPair D_8012F13C[16] __attribute__((section(".data"))) = {0};

extern StateValueHistory D_8013BE30;
extern u8 D_8013AFCD __attribute__((section(".data")));
extern u8 D_8013B47C;

INCLUDE_ASM("asm/nonmatchings/game/unk_8007CC78", func_8007CC78);

INCLUDE_ASM("asm/nonmatchings/game/unk_8007CC78", func_8007D47C);

INCLUDE_ASM("asm/nonmatchings/game/unk_8007CC78", func_8007D50C);

INCLUDE_ASM("asm/nonmatchings/game/unk_8007CC78", func_8007D5C8);

INCLUDE_ASM("asm/nonmatchings/game/unk_8007CC78", func_8007DE28);

INCLUDE_ASM("asm/nonmatchings/game/unk_8007CC78", func_8007E004);

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

INCLUDE_ASM("asm/nonmatchings/game/unk_8007CC78", func_8007F4BC);

INCLUDE_ASM("asm/nonmatchings/game/unk_8007CC78", func_8007F630);

INCLUDE_ASM("asm/nonmatchings/game/unk_8007CC78", func_80080114);

INCLUDE_ASM("asm/nonmatchings/game/unk_8007CC78", func_800801A0);

INCLUDE_ASM("asm/nonmatchings/game/unk_8007CC78", func_80080BC0);

INCLUDE_ASM("asm/nonmatchings/game/unk_8007CC78", func_80080C64);

INCLUDE_ASM("asm/nonmatchings/game/unk_8007CC78", func_80080FB4);

INCLUDE_ASM("asm/nonmatchings/game/unk_8007CC78", func_8008104C);

INCLUDE_ASM("asm/nonmatchings/game/unk_8007CC78", func_800810F0);

INCLUDE_ASM("asm/nonmatchings/game/unk_8007CC78", func_800811E0);

INCLUDE_ASM("asm/nonmatchings/game/unk_8007CC78", func_80081244);

INCLUDE_ASM("asm/nonmatchings/game/unk_8007CC78", func_800812B4);

INCLUDE_ASM("asm/nonmatchings/game/unk_8007CC78", func_80081318);

INCLUDE_ASM("asm/nonmatchings/game/unk_8007CC78", func_80081380);

INCLUDE_ASM("asm/nonmatchings/game/unk_8007CC78", func_80081400);

INCLUDE_ASM("asm/nonmatchings/game/unk_8007CC78", func_800815D0);

INCLUDE_ASM("asm/nonmatchings/game/unk_8007CC78", func_800816D8);

INCLUDE_ASM("asm/nonmatchings/game/unk_8007CC78", func_80081AC8);

INCLUDE_ASM("asm/nonmatchings/game/unk_8007CC78", func_80081BAC);

INCLUDE_ASM("asm/nonmatchings/game/unk_8007CC78", func_80081BFC);

INCLUDE_ASM("asm/nonmatchings/game/unk_8007CC78", func_80081DF0);

INCLUDE_ASM("asm/nonmatchings/game/unk_8007CC78", func_80082690);

INCLUDE_ASM("asm/nonmatchings/game/unk_8007CC78", func_80082A30);

INCLUDE_ASM("asm/nonmatchings/game/unk_8007CC78", func_80082CE0);

INCLUDE_ASM("asm/nonmatchings/game/unk_8007CC78", func_80083028);

ByteTriplet *lookupByteTriplet(ByteTriplet *records, u8 *indices, s16 index) {
    return &records[indices[index]];
}

INCLUDE_ASM("asm/nonmatchings/game/unk_8007CC78", func_80083180);

INCLUDE_ASM("asm/nonmatchings/game/unk_8007CC78", func_80083574);

INCLUDE_ASM("asm/nonmatchings/game/unk_8007CC78", func_80083968);

INCLUDE_ASM("asm/nonmatchings/game/unk_8007CC78", func_80083C3C);

INCLUDE_ASM("asm/nonmatchings/game/unk_8007CC78", func_80084254);

INCLUDE_ASM("asm/nonmatchings/game/unk_8007CC78", func_8008432C);

INCLUDE_ASM("asm/nonmatchings/game/unk_8007CC78", func_800843DC);

INCLUDE_ASM("asm/nonmatchings/game/unk_8007CC78", func_800846F4);

INCLUDE_ASM("asm/nonmatchings/game/unk_8007CC78", func_80084970);

INCLUDE_ASM("asm/nonmatchings/game/unk_8007CC78", func_8008573C);

INCLUDE_ASM("asm/nonmatchings/game/unk_8007CC78", func_800857C4);

INCLUDE_ASM("asm/nonmatchings/game/unk_8007CC78", func_80086584);

INCLUDE_ASM("asm/nonmatchings/game/unk_8007CC78", func_8008690C);

INCLUDE_ASM("asm/nonmatchings/game/unk_8007CC78", func_80086BCC);

INCLUDE_ASM("asm/nonmatchings/game/unk_8007CC78", func_80086C34);

INCLUDE_ASM("asm/nonmatchings/game/unk_8007CC78", func_80086DEC);

s32 isValue16(s32 value) {
    return value == 16;
}

s16 hasStateValueChanged(s16 index) {
    return D_8013BE30.current[index] != D_8013BE30.previous[index];
}

INCLUDE_ASM("asm/nonmatchings/game/unk_8007CC78", func_80087030);

INCLUDE_ASM("asm/nonmatchings/game/unk_8007CC78", func_800873DC);

INCLUDE_ASM("asm/nonmatchings/game/unk_8007CC78", func_80087464);

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

INCLUDE_ASM("asm/nonmatchings/game/unk_8007CC78", func_80089990);

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

INCLUDE_ASM("asm/nonmatchings/game/unk_8007CC78", func_80089E34);

INCLUDE_ASM("asm/nonmatchings/game/unk_8007CC78", func_80089EC0);

INCLUDE_ASM("asm/nonmatchings/game/unk_8007CC78", func_8008A058);

INCLUDE_ASM("asm/nonmatchings/game/unk_8007CC78", func_8008B178);

INCLUDE_ASM("asm/nonmatchings/game/unk_8007CC78", func_8008BCDC);

INCLUDE_ASM("asm/nonmatchings/game/unk_8007CC78", func_8008BEA4);

INCLUDE_ASM("asm/nonmatchings/game/unk_8007CC78", func_8008BF20);

INCLUDE_ASM("asm/nonmatchings/game/unk_8007CC78", func_8008BFF0);

INCLUDE_ASM("asm/nonmatchings/game/unk_8007CC78", func_8008C05C);

INCLUDE_ASM("asm/nonmatchings/game/unk_8007CC78", func_8008C0CC);

INCLUDE_ASM("asm/nonmatchings/game/unk_8007CC78", func_8008C1E0);

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

INCLUDE_ASM("asm/nonmatchings/game/unk_8007CC78", func_8008C568);

INCLUDE_ASM("asm/nonmatchings/game/unk_8007CC78", func_8008C5CC);
