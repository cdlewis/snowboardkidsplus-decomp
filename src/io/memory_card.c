#include "../engine/resource_services_internal.h"
#include "game/engine/system_runtime.h"
#include "game/io/memory_card.h"
#include "game/menu/selected_records.h"
#include "game/save/save_data.h"
#include "game/scene/scene_events.h"
#include "psx/libgpu.h"
#include "psx/libmcrd.h"

extern s16 gSelectedCourseIndex __attribute__((section(".data")));

char *gMemoryCardFileNames[5] __attribute__((section(".data.memory_card_file_names"))) = {
    D_800101B8, D_800101D0, D_800101E8, D_80010200, D_80010218
};

MemoryCardSaveTitle *gMemoryCardSaveTitles[5] __attribute__((section(".data.memory_card_tables"))) = {
    &D_80010230, &D_80010274, &D_800102B8, &D_800102FC, &D_80010340
};

u32 gMemoryCardSaveCapacities[5] __attribute__((section(".data.memory_card_tables"))) = {
    0xA80, 0x400, 0xB00, 0x8880, 0x2000
};

MemoryCardSaveHeader gMemoryCardSaveHeader __attribute__((section(".data.memory_card_save_header"))) = {0};

const u32 gMemoryCardAvailabilityTablePadding
    __attribute__((section(".rodata.memory_card_availability_padding"))) = 0;

u8 D_8013B608[2] __attribute__((section(".data.memory_card_poll_flags"))) = {0};
u8 gMemoryCardAcceptRetryCounts[2] __attribute__((section(".data.memory_card_accept_retry_counts"))) = {0};

void initializeMemoryCardSaveHeader(void) {
    u16 i;
    MemCardInit(0);
    MemCardStart();
    gMemoryCardSaveHeader.magic[0] = 'S';
    gMemoryCardSaveHeader.magic[1] = 'C';
    gMemoryCardSaveHeader.iconDisplay = 0x13;
    for (i = 0; i < 28; i++) {
        gMemoryCardSaveHeader.reserved[i] = 0;
    }
}

void captureMemoryCardSaveIcon(void) {
    RECT paletteRect;
    RECT iconRect;
    s16 icon;

    if ((s16)getGlobalFlagMask(0)) {
        icon = 2;
    } else if ((s16)getGlobalFlagMask(1)) {
        icon = 1;
    } else {
        icon = 0;
    }
    switch (icon) {
    case 0:
        paletteRect.y = 0xF4;
        iconRect.x = 0x34C;
        break;
    case 1:
        paletteRect.y = 0xF5;
        iconRect.x = 0x350;
        break;
    case 2:
        paletteRect.y = 0xF6;
        iconRect.x = 0x354;
        break;
    }
    paletteRect.x = 0x310;
    paletteRect.w = 16;
    paletteRect.h = 1;
    iconRect.y = 0x1D0;
    iconRect.w = 4;
    iconRect.h = 48;
    StoreImage(&paletteRect, (u32 *)gMemoryCardSaveHeader.iconPalette);
    StoreImage(&iconRect, (u32 *)gMemoryCardSaveHeader.icons);
}

void prepareMemoryCardSave(u8 slot, void *source, u32 size) {
    u8 blocks;
    u32 i;
    u32 checksum;
    gMemoryCardSelectedFileName = gMemoryCardFileNames[slot];
    blocks = gMemoryCardSaveCapacities[slot] / 8192;
    if (gMemoryCardSaveCapacities[slot] % 8192) blocks++;
    gMemoryCardSaveHeader.blockCount = blocks;
    gMemoryCardSaveHeader.title = *gMemoryCardSaveTitles[slot];
    memcpy(gMemoryCardSaveHeader.payload, source, size);
    for (i = size; i < gMemoryCardSaveCapacities[slot] - 0x210; i++) {
        gMemoryCardSaveHeader.payload[i] = 0;
    }
    i = 0;
    checksum = 0;
    gMemoryCardSaveHeader.payloadSize = size;
    gMemoryCardSaveHeader.version = 0;
    for (; i < size; i++) checksum += gMemoryCardSaveHeader.payload[i];
    gMemoryCardSaveHeader.checksum = checksum;
    D_8013AF48 = slot;
}

void selectMemoryCardLoadFile(u8 slot, void *destination) {
    switch (slot) {
    case 1:
    case 2:
    case 3:
        gMemoryCardFileNames[slot][18] = (gSelectedCourseIndex / 10) | 0x30;
        gMemoryCardFileNames[slot][19] = (gSelectedCourseIndex % 10) | 0x30;
        break;
    case 4:
        gMemoryCardFileNames[4][18] = (gSelectedUnlockIndex / 10) | 0x30;
        gMemoryCardFileNames[4][19] = (gSelectedUnlockIndex % 10) | 0x30;
        break;
    }
    gMemoryCardLoadDestination = destination;
    D_8013AF48 = slot;
    gMemoryCardSelectedFileName = gMemoryCardFileNames[slot];
}

s16 validateAndCopyMemoryCardSave(void) {
    u32 i;
    u32 checksum;

    if (gMemoryCardSaveHeader.version != 0) {
        return 0;
    }
    i = 0;
    checksum = 0;
    for (; i < gMemoryCardSaveHeader.payloadSize; i++) {
        checksum += gMemoryCardSaveHeader.payload[i];
    }
    if (gMemoryCardSaveHeader.checksum != checksum) {
        return 0;
    }
    memcpy(gMemoryCardLoadDestination, gMemoryCardSaveHeader.payload,
           gMemoryCardSaveHeader.payloadSize);
    return 1;
}

void readMemoryCardDirectory(void) {
    MemCardGetDirentry(gMemoryCardChannel, D_8013AF3C, gMemoryCardDirectoryEntries[gMemoryCardPortIndex],
                       &gMemoryCardDirectoryEntryCounts[gMemoryCardPortIndex], 0, 15);
}

s32 memoryCardFileExists(void) {
    s32 i;
    for (i = 0; i < gMemoryCardDirectoryEntryCounts[gMemoryCardPortIndex]; i++) {
        if (strcmp(gMemoryCardDirectoryEntries[gMemoryCardPortIndex][i].name, gMemoryCardSelectedFileName) == 0) {
            return 1;
        }
    }
    return 0;
}

s32 memoryCardIsFullWithoutSelectedFile(void) {
    s16 i = 0;
    s16 blocks = 0;
    s32 size;
    s32 wholeBlocks;
    s32 total;
    for (; i < gMemoryCardDirectoryEntryCounts[gMemoryCardPortIndex]; i++) {
        size = gMemoryCardDirectoryEntries[gMemoryCardPortIndex][i].size;
        wholeBlocks = size / 8192;
        total = blocks + ((size % 8192 ? 1 : 0) + wholeBlocks);
        blocks = total;
    }
    if (blocks < 15) {
        return 0;
    }
    for (i = 0; i < gMemoryCardDirectoryEntryCounts[gMemoryCardPortIndex]; i++) {
        if (strcmp(gMemoryCardDirectoryEntries[gMemoryCardPortIndex][i].name, gMemoryCardSelectedFileName) == 0) {
            return 0;
        }
    }
    return 1;
}

s32 tryAcceptMemoryCard(void) {
    if (MemCardSync(1, 0, 0) == -1) {
        MemCardAccept(gMemoryCardChannel);
        return 1;
    }
    return 0;
}

s32 pollMemoryCardAvailability(void) {
    s32 command;
    s32 result;

    D_8013B608[gMemoryCardPortIndex] = 1;
    switch (MemCardSync(1, &command, &result)) {
    case -1:
        MemCardExist(gMemoryCardChannel);
        gMemoryCardAcceptRetryCounts[gMemoryCardPortIndex] = 0;
        break;
    case 0:
        return 0;
    case 1:
        switch (command) {
        case 1:
            if (result != 1) {
                if (result == 0) {
                    goto checkExistingCard;
                }
                if (result == 2) {
                    goto unformattedCard;
                }
                if (result != 3) {
                    return 0;
                }
                goto newCard;
            checkExistingCard:
                if ((s16)memoryCardIsFullWithoutSelectedFile()) {
                    return 7;
                }
                if (!(s16)memoryCardFileExists()) {
                    return 8;
                }
                if (D_8013B60C[gMemoryCardPortIndex]) {
                    return 2;
                }
                return 1;
            newCard:
                D_8013B60C[gMemoryCardPortIndex] = 1;
                MemCardAccept(gMemoryCardChannel);
                return 5;
            }
            return 3;
        case 2:
            switch (result) {
            case 0:
                if ((s16)memoryCardIsFullWithoutSelectedFile()) {
                    return 7;
                }
                if (!(s16)memoryCardFileExists()) {
                    return 8;
                }
                if (D_8013B60C[gMemoryCardPortIndex]) {
                    return 2;
                }
                return 1;
            case 1:
                return 3;
            case 3:
                D_8013B60C[gMemoryCardPortIndex] = 1;
                readMemoryCardDirectory();
                if ((s16)memoryCardIsFullWithoutSelectedFile()) {
                    return 7;
                }
                if (!(s16)memoryCardFileExists()) {
                    return 8;
                }
                if (D_8013B60C[gMemoryCardPortIndex]) {
                    return 2;
                }
                return 1;
            case 2:
            unformattedCard:
                return 4;
            case 4:
                if (gMemoryCardAcceptRetryCounts[gMemoryCardPortIndex] < 10) {
                    gMemoryCardAcceptRetryCounts[gMemoryCardPortIndex]++;
                    MemCardAccept(gMemoryCardChannel);
                    return 5;
                }
                gMemoryCardAcceptRetryCounts[gMemoryCardPortIndex] = 0;
                gMemoryCardDirectoryEntryCounts[gMemoryCardPortIndex] = 0;
                return 6;
            }
            break;
        default:
            return 0;
        }
        break;
    default:
        return 0;
    }
    return 0;
}
