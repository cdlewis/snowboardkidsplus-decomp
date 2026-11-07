#include "save_record_tasks_internal.h"

u16 gTaskShadowedText[14] __attribute__((section(".data.task_shadowed_text"))) = {
    0x001A, 0x001F, 0x001D, 0x0012, 0x8020, 0x8020, 0x000C,
    0x001F, 0x001E, 0x001E, 0x0019, 0x0018, 0, 0
};

void repeatByteStore(u8 *destination, s32 value, s32 count) {
    s16 i;

    /* The original loop writes repeatedly to the same byte. */
    for (i = 0; i < count; i++) {
        *destination = value;
    }
}

INCLUDE_ASM("asm/nonmatchings/menu/save_record_tasks", func_800897D0);

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

INCLUDE_ASM("asm/nonmatchings/menu/save_record_tasks", func_80089A0C);

INCLUDE_ASM("asm/nonmatchings/menu/save_record_tasks", func_80089A98);

u32 calculateByteSum(const u8 *data, u32 size) {
    u32 sum;
    u32 i;

    sum = 0;
    for (i = 0; i < size; i++) {
        sum += *data++;
    }
    return sum;
}

INCLUDE_ASM("asm/nonmatchings/menu/save_record_tasks", func_80089B30);

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

INCLUDE_ASM("asm/nonmatchings/menu/save_record_tasks", func_80089EC0);

INCLUDE_ASM("asm/nonmatchings/menu/save_record_tasks", func_8008A058);

INCLUDE_ASM("asm/nonmatchings/menu/save_record_tasks", func_8008B178);

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

INCLUDE_ASM("asm/nonmatchings/menu/save_record_tasks", func_8008C290);

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
