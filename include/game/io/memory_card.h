#pragma once

#include "common.h"
#include "psx/libmcrd.h"

typedef struct MemoryCardSaveTitle {
    u8 bytes[64];
} MemoryCardSaveTitle;

typedef struct MemoryCardSaveHeader {
    u8 magic[2];
    u8 iconDisplay;
    u8 blockCount;
    MemoryCardSaveTitle title;
    u8 reserved[28];
    u16 iconPalette[16];
    u8 icons[3][128];
    u32 version;
    u32 payloadSize;
    u32 checksum;
    u32 unk20C;
    u8 payload[0x870];
} MemoryCardSaveHeader;

extern MemoryCardSaveHeader gMemoryCardSaveHeader __attribute__((section(".data.memory_card_save_header")));
extern void *gMemoryCardLoadDestination;
extern s32 gMemoryCardDirectoryEntryCounts[2];
extern struct DIRENTRY gMemoryCardDirectoryEntries[2][15];
extern s16 gMemoryCardPortIndex;
extern s32 gMemoryCardChannel;
extern u8 gMemoryCardSelectedFileType;
extern char *gMemoryCardSelectedFileName;
extern u8 gMemoryCardCreateFilePending[2];
extern u8 gMemoryCardChanged[2] __attribute__((section(".bss")));
extern u32 gMemoryCardSaveCapacities[5];

void selectMemoryCardLoadFile(u8 slot, void *destination);
void prepareMemoryCardSave(u8 slot, void *source, u32 size);
s16 validateAndCopyMemoryCardSave(void);
s32 pollMemoryCardRead(void);
s32 pollMemoryCardWrite(void);
void readMemoryCardDirectory(void);
void resetMemoryCardTaskState(void);
s32 formatMemoryCard(void);
void setMemoryCardChannel(s32 channel);
s32 startMemoryCardReadStatusTask(void *buffer);
s32 startMemoryCardReadTask(void);
s32 startMemoryCardSaveStatusTask(void *buffer, u32 size);
s32 startMemoryCardWriteTask(void);

typedef char MemoryCardSaveHeaderSizeCheck[(sizeof(MemoryCardSaveHeader) == 0xA80) ? 1 : -1];
