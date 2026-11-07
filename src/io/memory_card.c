#include "../engine/resource_services_internal.h"

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
    D_8013AF40 = gMemoryCardFileNames[slot];
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
        gMemoryCardFileNames[slot][18] = (selectedCourseIndexAbsolute / 10) | 0x30;
        gMemoryCardFileNames[slot][19] = (selectedCourseIndexAbsolute % 10) | 0x30;
        break;
    case 4:
        gMemoryCardFileNames[4][18] = (D_8013AF3A / 10) | 0x30;
        gMemoryCardFileNames[4][19] = (D_8013AF3A % 10) | 0x30;
        break;
    }
    gMemoryCardLoadDestination = destination;
    D_8013AF48 = slot;
    D_8013AF40 = gMemoryCardFileNames[slot];
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
    MemCardGetDirentry(D_8013AF34, D_8013AF3C, D_8013B700[D_8013AF38],
                       &D_8013B600[D_8013AF38], 0, 15);
}

s32 memoryCardFileExists(void) {
    s32 i;
    for (i = 0; i < D_8013B600[D_8013AF38]; i++) {
        if (strcmp(D_8013B700[D_8013AF38][i].name, D_8013AF40) == 0) {
            return 1;
        }
    }
    return 0;
}

INCLUDE_ASM("asm/nonmatchings/io/memory_card", func_8001907C);

s32 tryAcceptMemoryCard(void) {
    if (MemCardSync(1, 0, 0) == -1) {
        MemCardAccept(D_8013AF34);
        return 1;
    }
    return 0;
}

INCLUDE_ASM("asm/nonmatchings/io/memory_card", func_80019288);
