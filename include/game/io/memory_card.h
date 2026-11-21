#pragma once

#include "common.h"

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

extern MemoryCardSaveHeader gMemoryCardSaveHeader;
extern s16 D_8013AF38;
extern s32 D_8013AF34;
extern u8 D_8013AF48;
extern char *D_8013AF40;
extern u8 D_8013B608[2];
extern u8 D_8013B60C[2] __attribute__((section(".bss")));
extern u32 gMemoryCardSaveCapacities[5];

void prepareMemoryCardSave(u8 slot, void *source, u32 size);
s16 validateAndCopyMemoryCardSave(void);
s32 pollMemoryCardRead(void);
s32 pollMemoryCardWrite(void);
void readMemoryCardDirectory(void);
void resetMemoryCardTaskState(void);
s32 func_8001983C(void);
void func_800198C4(s32 channel);
s32 func_800198EC(void *buffer);
s32 startMemoryCardReadTask(void);
s32 func_80019AF4(void *buffer, s32 size);
s32 startMemoryCardWriteTask(void);

typedef char MemoryCardSaveHeaderSizeCheck[(sizeof(MemoryCardSaveHeader) == 0xA80) ? 1 : -1];
