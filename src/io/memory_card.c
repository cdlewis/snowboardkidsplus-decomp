#include "../engine/resource_services_internal.h"
#include "game/engine/system_runtime.h"
#include "game/io/memory_card.h"
#include "game/menu/selected_records.h"
#include "game/save/save_data.h"
#include "game/scene/scene_events.h"
#include "psx/libgpu.h"
#include "psx/libmcrd.h"

/* Packed 4-bit pixels: each eight-byte row holds sixteen palette indices. */
typedef struct MemoryCardIconTemplate {
    u16 palette[16];
    u8 frames[3][16][8];
} MemoryCardIconTemplate;

s32 gMemoryCardChannel __attribute__((section(".sdata.memory_card_channel"))) = 0;
s16 gMemoryCardPortIndex __attribute__((section(".sdata.memory_card_channel"))) = 0;
char gMemoryCardDirectorySearchPattern[2] __attribute__((section(".sdata.memory_card_search"), aligned(2))) = "*";
char *gMemoryCardSelectedFileName __attribute__((section(".sdata.memory_card_selection"))) = NULL;
void *gMemoryCardLoadDestination __attribute__((section(".sdata.memory_card_selection"))) = NULL;
u8 gMemoryCardSelectedFileType __attribute__((section(".sdata.memory_card_selection"))) = 0;
const u8 gMemoryCardSelectionPadding0 __attribute__((section(".sdata.memory_card_selection"))) = 0;
const u8 gMemoryCardSelectionPadding1 __attribute__((section(".sdata.memory_card_selection"))) = 0;
const u8 gMemoryCardSelectionPadding2 __attribute__((section(".sdata.memory_card_selection"))) = 0;

MemoryCardIconTemplate gMemoryCardIconTemplate
    __attribute__((section(".data.memory_card_icon_template"), aligned(4))) = {
    {
        0x0000, 0x7FFF, 0x62D6, 0x001E, 0x113D, 0x21DD, 0x329D, 0x2618,
        0x1D93, 0x152E, 0x7B20, 0x6200, 0x4520, 0x0421, 0x03E0, 0x03E0,
    },
    {
        {
            {0x00, 0x00, 0x00, 0x00, 0x00, 0xDD, 0x00, 0xDD},
            {0x00, 0x00, 0x00, 0xDD, 0xDD, 0xD2, 0xD0, 0xD2},
            {0x00, 0x00, 0xDD, 0x78, 0x77, 0xD8, 0x2D, 0x0D},
            {0x00, 0xD0, 0x68, 0x66, 0x66, 0x86, 0xD2, 0x00},
            {0x00, 0x8D, 0x66, 0x66, 0x66, 0x16, 0xD1, 0x00},
            {0x00, 0x2D, 0x31, 0x13, 0x11, 0x61, 0xD8, 0x00},
            {0x00, 0x6D, 0x66, 0x66, 0x66, 0x66, 0x86, 0x0D},
            {0xD0, 0xD8, 0x66, 0x66, 0xD6, 0x66, 0x66, 0x0D},
            {0xD0, 0x47, 0x44, 0x44, 0x44, 0x65, 0x66, 0xD6},
            {0x7D, 0x66, 0x45, 0x44, 0x65, 0x66, 0x66, 0xD9},
            {0x9D, 0x69, 0x66, 0x86, 0x66, 0x66, 0x76, 0x0D},
            {0x66, 0x97, 0xAC, 0x8D, 0x66, 0x66, 0xD7, 0x00},
            {0xC8, 0xBC, 0xCA, 0x66, 0x78, 0x76, 0xD9, 0x00},
            {0xBC, 0xBB, 0x9B, 0x99, 0x89, 0x67, 0x76, 0x0D},
            {0xAA, 0xBB, 0x7C, 0x79, 0x66, 0x66, 0x66, 0xD7},
            {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
        },
        {
            {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
            {0xDD, 0x0D, 0x00, 0xDD, 0xDD, 0xDD, 0x00, 0x00},
            {0xAA, 0xDC, 0xD0, 0x78, 0x66, 0x76, 0x0D, 0x00},
            {0xAA, 0xCA, 0x2D, 0x31, 0x13, 0x11, 0xD6, 0x00},
            {0xAA, 0xBA, 0xDC, 0x66, 0x66, 0x6D, 0x61, 0x0D},
            {0xAA, 0xBB, 0x4C, 0x44, 0x44, 0x55, 0x16, 0x0D},
            {0xBC, 0xBB, 0x6B, 0x66, 0x67, 0x66, 0x66, 0xD2},
            {0xC8, 0xBC, 0xCB, 0x76, 0x66, 0x66, 0x66, 0xD6},
            {0x66, 0x97, 0xBC, 0x76, 0x66, 0x66, 0x66, 0xD6},
            {0x76, 0x6D, 0x88, 0x66, 0x66, 0x66, 0x76, 0xD8},
            {0xDD, 0x8D, 0x88, 0x66, 0x66, 0x76, 0x87, 0xD2},
            {0x00, 0xD0, 0x89, 0x78, 0x77, 0x77, 0xD8, 0xD2},
            {0x00, 0xD0, 0xDD, 0x99, 0x78, 0x66, 0xD7, 0x0D},
            {0x00, 0x8D, 0x67, 0x66, 0x66, 0x66, 0x76, 0x0D},
            {0xD0, 0x78, 0x66, 0x66, 0x66, 0x66, 0x66, 0xD7},
            {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
        },
        {
            {0x00, 0x00, 0x00, 0x00, 0x00, 0xDD, 0x00, 0xDD},
            {0x00, 0x00, 0x00, 0xDD, 0xDD, 0xD2, 0xD0, 0xD2},
            {0x00, 0x00, 0xDD, 0x78, 0x77, 0xD8, 0x2D, 0x0D},
            {0x00, 0xD0, 0x68, 0x66, 0x66, 0x86, 0xD2, 0x00},
            {0x00, 0x8D, 0x66, 0x66, 0x66, 0x16, 0xD1, 0x00},
            {0x00, 0x2D, 0x31, 0x13, 0x11, 0x61, 0xD8, 0x00},
            {0x00, 0x6D, 0x66, 0x66, 0x66, 0x66, 0x86, 0x0D},
            {0xD0, 0xD8, 0x66, 0x66, 0xD6, 0x66, 0x66, 0x0D},
            {0xD0, 0x47, 0x44, 0x44, 0x44, 0x65, 0x66, 0xD6},
            {0x7D, 0x66, 0x45, 0x44, 0x65, 0x66, 0x66, 0xD9},
            {0x8D, 0x66, 0x66, 0x66, 0x66, 0x66, 0x76, 0x0D},
            {0xD0, 0x67, 0xD6, 0x6D, 0x66, 0x66, 0xD7, 0x00},
            {0x00, 0x9D, 0xD7, 0x6D, 0x66, 0x76, 0xD9, 0x00},
            {0x00, 0x2D, 0x21, 0x99, 0x89, 0x67, 0x76, 0x0D},
            {0xD0, 0x18, 0x11, 0x77, 0x66, 0x66, 0x66, 0xD7},
            {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
        },
    }
};

extern s16 gSelectedCourseIndex __attribute__((section(".data")));

char gMemoryCardGameFileName[24] __attribute__((section(".data.memory_card_file_templates"), aligned(4))) = "BISLPS-01823-SKP_GAM";

char gMemoryCardGhostFileName[24] __attribute__((section(".data.memory_card_file_templates"), aligned(4))) = "BISLPS-01823-SKP_G00";

char gMemoryCardPlacedObjectFileName[24] __attribute__((section(".data.memory_card_file_templates"), aligned(4))) = "BISLPS-01823-SKP_S00";

char gMemoryCardTerrainCollisionFileName[24] __attribute__((section(".data.memory_card_file_templates"), aligned(4))) = "BISLPS-01823-SKP_H00";

char gMemoryCardDigicomicFileName[24] __attribute__((section(".data.memory_card_file_templates"), aligned(4))) = "BISLPS-01823-SKP_D00";

/* Card titles contain 32 full-width Shift-JIS characters without a terminator. */
MemoryCardSaveTitle gMemoryCardGameSaveTitle __attribute__((section(".data.memory_card_title_templates"), aligned(4))) = {
    "\x82\x72\x82\x8E\x82\x8F\x82\x82\x82\x8F\x82\x97\x82\x6A\x82\x89"
    "\x82\x84\x82\x93\x82\x6F\x82\x8C\x82\x95\x82\x93\x81\x40\x81\x69"
    "\x83\x51\x81\x5B\x83\x80\x83\x66\x81\x7C\x83\x5E\x81\x6A\x81\x40"
    "\x81\x40\x81\x40\x81\x40\x81\x40\x81\x40\x81\x40\x81\x40\x81\x40"
};

const u32 gMemoryCardGameSaveTitlePadding __attribute__((section(".data.memory_card_title_templates"))) = 0;

MemoryCardSaveTitle gMemoryCardGhostSaveTitle __attribute__((section(".data.memory_card_title_templates"), aligned(4))) = {
    "\x82\x72\x82\x8E\x82\x8F\x82\x82\x82\x8F\x82\x97\x82\x6A\x82\x89"
    "\x82\x84\x82\x93\x82\x6F\x82\x8C\x82\x95\x82\x93\x81\x40\x81\x69"
    "\x83\x53\x81\x5B\x83\x58\x83\x67\x83\x66\x81\x7C\x83\x5E\x81\x6A"
    "\x81\x40\x82\x72\x82\x73\x82\x60\x82\x66\x82\x64\x82\x4F\x82\x4F"
};

const u32 gMemoryCardGhostSaveTitlePadding __attribute__((section(".data.memory_card_title_templates"))) = 0;

MemoryCardSaveTitle gMemoryCardPlacedObjectSaveTitle __attribute__((section(".data.memory_card_title_templates"), aligned(4))) = {
    "\x82\x72\x82\x8E\x82\x8F\x82\x82\x82\x8F\x82\x97\x82\x6A\x82\x89"
    "\x82\x84\x82\x93\x82\x6F\x82\x8C\x82\x95\x82\x93\x81\x40\x81\x69"
    "\x90\xDD\x92\x75\x95\xA8\x82\xCC\x83\x66\x81\x5B\x83\x5E\x81\x6A"
    "\x81\x40\x82\x72\x82\x73\x82\x60\x82\x66\x82\x64\x82\x4F\x82\x4F"
};

const u32 gMemoryCardPlacedObjectSaveTitlePadding __attribute__((section(".data.memory_card_title_templates"))) = 0;

MemoryCardSaveTitle gMemoryCardTerrainCollisionSaveTitle __attribute__((section(".data.memory_card_title_templates"), aligned(4))) = {
    "\x82\x72\x82\x8E\x82\x8F\x82\x82\x82\x8F\x82\x97\x82\x6A\x82\x89"
    "\x82\x84\x82\x93\x82\x6F\x82\x8C\x82\x95\x82\x93\x81\x40\x81\x69"
    "\x93\x79\x92\x6E\x93\x96\x82\xE8\x83\x66\x81\x7C\x83\x5E\x81\x6A"
    "\x81\x40\x82\x72\x82\x73\x82\x60\x82\x66\x82\x64\x82\x4F\x82\x4F"
};

const u32 gMemoryCardTerrainCollisionSaveTitlePadding __attribute__((section(".data.memory_card_title_templates"))) = 0;

MemoryCardSaveTitle gMemoryCardDigicomicSaveTitle __attribute__((section(".data.memory_card_title_templates"), aligned(4))) = {
    "\x82\x72\x82\x8E\x82\x8F\x82\x82\x82\x8F\x82\x97\x82\x6A\x82\x89"
    "\x82\x84\x82\x93\x82\x6F\x82\x8C\x82\x95\x82\x93\x81\x40\x81\x69"
    "\x83\x66\x83\x57\x83\x52\x83\x7E\x83\x66\x81\x7C\x83\x5E\x81\x6A"
    "\x81\x40\x82\x72\x82\x73\x82\x60\x82\x66\x82\x64\x82\x4F\x82\x4F"
};

const u32 gMemoryCardDigicomicSaveTitlePadding __attribute__((section(".data.memory_card_title_templates"))) = 0;

const u32 gMemoryCardTitleTemplatesPadding __attribute__((section(".data.memory_card_title_templates"))) = 0;

char *gMemoryCardFileNames[5] __attribute__((section(".data.memory_card_file_names"))) = {
    gMemoryCardGameFileName, gMemoryCardGhostFileName, gMemoryCardPlacedObjectFileName, gMemoryCardTerrainCollisionFileName, gMemoryCardDigicomicFileName
};

MemoryCardSaveTitle *gMemoryCardSaveTitles[5] __attribute__((section(".data.memory_card_tables"))) = {
    &gMemoryCardGameSaveTitle, &gMemoryCardGhostSaveTitle, &gMemoryCardPlacedObjectSaveTitle, &gMemoryCardTerrainCollisionSaveTitle, &gMemoryCardDigicomicSaveTitle
};

u32 gMemoryCardSaveCapacities[5] __attribute__((section(".data.memory_card_tables"))) = {
    0xA80, 0x400, 0xB00, 0x8880, 0x2000
};

MemoryCardSaveHeader gMemoryCardSaveHeader __attribute__((section(".data.memory_card_save_header"))) = {0};

const u32 gMemoryCardAvailabilityTablePadding
    __attribute__((section(".rodata.memory_card_availability_padding"))) = 0;

u8 gMemoryCardCreateFilePending[2] __attribute__((section(".data.memory_card_poll_flags"))) = {0};
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

void prepareMemoryCardSave(u8 fileType, const void *source, u32 size) {
    u8 blocks;
    u32 i;
    u32 checksum;
    gMemoryCardSelectedFileName = gMemoryCardFileNames[fileType];
    blocks = gMemoryCardSaveCapacities[fileType] / 8192;
    if (gMemoryCardSaveCapacities[fileType] % 8192) blocks++;
    gMemoryCardSaveHeader.blockCount = blocks;
    gMemoryCardSaveHeader.title = *gMemoryCardSaveTitles[fileType];
    memcpy(gMemoryCardSaveHeader.payload, source, size);
    for (i = size; i < gMemoryCardSaveCapacities[fileType] - 0x210; i++) {
        gMemoryCardSaveHeader.payload[i] = 0;
    }
    i = 0;
    checksum = 0;
    gMemoryCardSaveHeader.payloadSize = size;
    gMemoryCardSaveHeader.version = 0;
    for (; i < size; i++) checksum += gMemoryCardSaveHeader.payload[i];
    gMemoryCardSaveHeader.checksum = checksum;
    gMemoryCardSelectedFileType = fileType;
}

void selectMemoryCardLoadFile(u8 fileType, void *destination) {
    switch (fileType) {
    case 1:
    case 2:
    case 3:
        gMemoryCardFileNames[fileType][18] = (gSelectedCourseIndex / 10) | 0x30;
        gMemoryCardFileNames[fileType][19] = (gSelectedCourseIndex % 10) | 0x30;
        break;
    case 4:
        gMemoryCardFileNames[4][18] = (gSelectedUnlockIndex / 10) | 0x30;
        gMemoryCardFileNames[4][19] = (gSelectedUnlockIndex % 10) | 0x30;
        break;
    }
    gMemoryCardLoadDestination = destination;
    gMemoryCardSelectedFileType = fileType;
    gMemoryCardSelectedFileName = gMemoryCardFileNames[fileType];
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
    MemCardGetDirentry(gMemoryCardChannel, gMemoryCardDirectorySearchPattern, gMemoryCardDirectoryEntries[gMemoryCardPortIndex],
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

    gMemoryCardCreateFilePending[gMemoryCardPortIndex] = 1;
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
                if (gMemoryCardChanged[gMemoryCardPortIndex]) {
                    return 2;
                }
                return 1;
            newCard:
                gMemoryCardChanged[gMemoryCardPortIndex] = 1;
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
                if (gMemoryCardChanged[gMemoryCardPortIndex]) {
                    return 2;
                }
                return 1;
            case 1:
                return 3;
            case 3:
                gMemoryCardChanged[gMemoryCardPortIndex] = 1;
                readMemoryCardDirectory();
                if ((s16)memoryCardIsFullWithoutSelectedFile()) {
                    return 7;
                }
                if (!(s16)memoryCardFileExists()) {
                    return 8;
                }
                if (gMemoryCardChanged[gMemoryCardPortIndex]) {
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
