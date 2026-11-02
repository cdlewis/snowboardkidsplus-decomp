#pragma once

#include "psx/libcd.h"

/* Shared controller state belongs to the bios object, still in split data. */
extern u8 CD_status;
extern u8 CD_mode;
extern u8 CD_com;
extern CdlLOC CD_pos;
extern s32 CD_debug;
extern CdlCB CD_cbsync;
extern CdlCB CD_cbready;

s32 CD_init(void);
void CD_initintr(void);
s32 CD_initvol(void);
void CD_flush(void);
s32 CD_sync(s32 mode, u8 *result);
s32 CD_ready(s32 mode, u8 *result);
s32 CD_vol(CdlATV *volume);
s32 CD_getsector(void *destination, s32 size);
s32 CD_getsector2(void *destination, s32 size);
s32 CD_datasync(s32 mode);
