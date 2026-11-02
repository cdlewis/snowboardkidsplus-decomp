#pragma once

#include "psx/libgpu.h"
#include "psx/libetc.h"

/* Shared cache and dispatch storage are initialized by the GPU setup and
 * environment submission functions, which still use their original assembly.
 */
typedef struct GpuInfo {
    u8 version, unk1, level, reverse;
    s16 w, h;
    u8 unk8[4];
    DmaCallback drawSyncCb;
    DRAWENV draw;
    DISPENV disp;
} GpuInfo;

typedef struct GpuVtable {
    const char *version;
    s32 (*addque)(s32 (*execute)(u32, u32), u32 first, u32 second);
    s32 (*addque2)(s32 (*execute)(u32, u32), u32 first, s32 length, u32 second);
    s32 (*clr)(RECT *rect, u32 color);
    void (*ctl)(u32 command);
    s32 (*cwb)(u32 *data, s32 length);
    void (*cwc)(u32 *packets);
    s32 (*drs)(RECT *rect, u32 *data);
    s32 (*dws)(RECT *rect, u32 *data);
    s32 (*exeque)(void);
    s32 (*getctl)(s32 mode);
    s32 (*otc)(u32 *table, s32 length);
    s32 (*param)(s32 value);
    s32 (*reset)(s32 mode);
    u32 (*status)(void);
    s32 (*sync)(s32 mode);
} GpuVtable;

extern GpuInfo gGpuInfo;
extern GpuVtable *gGpuDispatch;
void *memcpy(void *destination, const void *source, u32 size);
s32 _getctl(s32 index);
u32 gpuGetMode(s32 dfe, s32 dtd, u16 tpage);
u32 gpuGetClipStart(s16 x, s16 y);
u32 gpuGetClipEnd(s16 x, s16 y);
u32 gpuGetOffset(s16 x, s16 y);
u32 gpuGetTextureWindow(RECT *rect);
