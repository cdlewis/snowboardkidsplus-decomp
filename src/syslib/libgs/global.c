#include "psx/libgs.h"

#define GLOBAL_DATA __attribute__((section(".data.gs_global")))

s32 GsADIVZ GLOBAL_DATA = 0;
s16 GsADIVW GLOBAL_DATA = 1023;
s16 GsADIVH GLOBAL_DATA = 511;

/* GLOBAL.OBJ ends its initialized data with eight unreferenced zero bytes. */
static u32 gsGlobalDataPadding[2] GLOBAL_DATA = {0, 0};
