#pragma once

#include "decomp/types.h"

/* Packed BCD location used by PSY-Q's CD APIs. */
typedef struct CdlLOC {
    u8 minute, second, sector, track;
} CdlLOC;

typedef struct CdlFILE {
    CdlLOC pos;
    u32 size;
    char name[16];
} CdlFILE;

typedef struct CdlATV {
    u8 val0, val1, val2, val3;
} CdlATV;

#define CdlNop 0x01
#define CdlSetloc 0x02
#define CdlSetmode 0x0E
#define CdlStatRead 0x20

#define CdlModeDA 1
#define CdlModeAP 2
#define CdlModeSize1 0x20
#define CdlModeRT 0x40
#define CdlModeSpeed 0x80
#define CdlModeStream 0x100

typedef void (*CdlCB)(u8 interrupt, u8 *result);

s32 CdControl(u8 command, u8 *param, u8 *result);
s32 CdRead2(s32 mode);
s32 CdControlB(u8 command, u8 *param, u8 *result);
s32 CdStatus(void);
s32 CdMode(void);
s32 CdLastCom(void);
CdlLOC *CdLastPos(void);
s32 CdReset(s32 mode);
s32 CdSetDebug(s32 level);
CdlCB CdSyncCallback(CdlCB callback);
CdlCB CdReadyCallback(CdlCB callback);
void CdFlush(void);
s32 CdSync(s32 mode, u8 *result);
s32 CdReady(s32 mode, u8 *result);
s32 CdMix(CdlATV *volume);
s32 CdGetSector(void *destination, s32 size);
s32 CdGetSector2(void *destination, s32 size);
s32 CdDataSync(s32 mode);
CdlLOC *CdIntToPos(s32 sector, CdlLOC *position);
s32 CdPosToInt(CdlLOC *position);

void StSetRing(u8 *addr, s32 size);
void StSetStream(u32 mode, u32 start, u32 end, void (*func1)(void), void (*func2)(void));

u32 StFreeRing(u32 *base);

s32 CdInit(void);
CdlFILE *CdSearchFile(CdlFILE *file, char *name);
