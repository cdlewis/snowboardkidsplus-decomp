#include "common.h"
#include "libgpu_private.h"
#include "psx/libetc.h"


/* Linked-list VRAM copy packet; this path omits SetDrawMove's cache flush. */
#define GPU_COPY_COMMAND 0x80000000
#define GPU_COPY_TAG 0x04FFFFFF

typedef struct GpuCopyPacket {
    u32 tag, command;
    u32 source, destination, size;
} GpuCopyPacket;

static GpuCopyPacket sGpuMovePacket
    __attribute__((section(".data.gpu_move_packet"), aligned(4))) = {
        GPU_COPY_TAG, GPU_COPY_COMMAND, 0, 0, 0
    };

#define GPU_REGISTERS __attribute__((section(".data.gpu_registers"), aligned(4)))
volatile u32 *GPU_DATA GPU_REGISTERS = (volatile u32 *)0x1F801810;
volatile u32 *GPU_STATUS GPU_REGISTERS = (volatile u32 *)0x1F801814;
volatile u32 *DMA2_MADR GPU_REGISTERS = (volatile u32 *)0x1F8010A0;
volatile u32 *DMA2_BCR GPU_REGISTERS = (volatile u32 *)0x1F8010A4;
volatile u32 *DMA2_CHCR GPU_REGISTERS = (volatile u32 *)0x1F8010A8;
volatile u32 *DMA6_MADR GPU_REGISTERS = (volatile u32 *)0x1F8010E0;
volatile u32 *DMA6_BCR GPU_REGISTERS = (volatile u32 *)0x1F8010E4;
volatile u32 *DMA6_CHCR GPU_REGISTERS = (volatile u32 *)0x1F8010E8;
volatile u32 *DPCR GPU_REGISTERS = (volatile u32 *)0x1F8010F0;

#define GPU_ALARM __attribute__((section(".data.gpu_alarm"), aligned(4)))
s32 gpuAlarmDeadline GPU_ALARM = 0;
volatile s32 gpuAlarmPollCount GPU_ALARM = 0;

extern int printf(const char *, ...);
int (*GPU_printf)(const char *, ...)
    __attribute__((section(".data.gpu_printf"), aligned(4))) = printf;

const char gpuSourceVersion[52] __attribute__((section(".rodata.gpu_reset_graph"), aligned(4))) =
    "$Id: sys.c,v 1.140 1998/01/12 07:52:27 noda Exp $";
const char gpuResetStateFormat[32] __attribute__((section(".rodata.gpu_reset_graph"), aligned(4))) =
    "ResetGraph:jtb=%08x,env=%08x\n";
const char gpuResetGraphFormat[20] __attribute__((section(".rodata.gpu_reset_graph"), aligned(4))) =
    "ResetGraph(%d)...\n";
const char gpuGraphQueueFormat[20] __attribute__((section(".rodata.gpu_graph_queue"), aligned(4))) =
    "SetGrapQue(%d)...\n";
const char gpuMoveImageName[12] __attribute__((section(".rodata.gpu_clear_otag"), aligned(4))) =
    "MoveImage";
const char gpuClearOTagFormat[24] __attribute__((section(".rodata.gpu_clear_otag"), aligned(4))) =
    "ClearOTag(%08x,%d)...\n";
const char gpuClearOTagReverseFormat[24] __attribute__((section(".rodata.gpu_clear_otag"), aligned(4))) =
    "ClearOTagR(%08x,%d)...\n";
const char gpuPutDispEnvFormat[24] __attribute__((section(".rodata.gpu_disp_env"), aligned(4))) =
    "PutDispEnv(%08x)...\n";
const char gpuQueueTimeoutFormat[52] __attribute__((section(".rodata.gpu_disp_env"), aligned(4))) =
    "GPU timeout:que=%d,stat=%08x,chcr=%08x,madr=%08x\n";

#define GPU_IMAGE_NAME __attribute__((section(".rodata.gpu_image_names"), aligned(4)))
const char gpuClearImageName[12] GPU_IMAGE_NAME = "ClearImage";
const char gpuClearImage2Name[12] GPU_IMAGE_NAME = "ClearImage2";
const char gpuLoadImageName[12] GPU_IMAGE_NAME = "LoadImage";
const char gpuStoreImageName[12] __attribute__((section(".rodata.gpu_image_names"), aligned(4))) = "StoreImage";
const char gpuDrawOTagFormat[20] __attribute__((section(".rodata.gpu_draw_otag"), aligned(4))) =
    "DrawOTag(%08x)...\n";
const char gpuPutDrawEnvFormat[24] __attribute__((section(".rodata.gpu_draw_otag"), aligned(4))) =
    "PutDrawEnv(%08x)...\n";
const char gpuDrawOTagEnvFormat[28] __attribute__((section(".rodata.gpu_draw_otag"), aligned(4))) =
    "DrawOTagEnv(%08x,&08x)...\n";
const char gpuGraphDebugFormat[44] __attribute__((section(".rodata.gpu_graph_debug"), aligned(4))) =
    "SetGraphDebug:level:%d,type:%d reverse:%d\n";
const char gpuDrawSyncCallbackFormat[28] __attribute__((section(".rodata.gpu_sync_callback"), aligned(4))) =
    "DrawSyncCallback(%08x)...\n";
const char gpuDisplayMaskFormat[20] __attribute__((section(".rodata.gpu_sync_callback"), aligned(4))) =
    "SetDispMask(%d)...\n";
const char gpuDrawSyncFormat[20] __attribute__((section(".rodata.gpu_sync_callback"), aligned(4))) =
    "DrawSync(%d)...\n";
const char gpuInvalidRectangleFormat[12] __attribute__((section(".rodata.gpu_rect_debug"), aligned(4))) = "%s:bad RECT";
const char gpuRectangleCoordinatesFormat[20] __attribute__((section(".rodata.gpu_rect_debug"), aligned(4))) = "(%d,%d)-(%d,%d)\n";
const char gpuOperationPrefixFormat[4] __attribute__((section(".rodata.gpu_rect_debug"), aligned(4))) = "%s:";
const char gpuLoadImage2Name[16] __attribute__((section(".rodata.gpu_load_image2"), aligned(4))) =
    "LoadImage2";

extern void set_alarm(void);
extern int get_alarm(void);
extern int VSync(int mode);
extern int _exeque(void);
extern u8 gpuControlCache[];

extern const char gpuDispatchTable[];
extern u16 gpuDefaultWidths[], gpuDefaultHeights[];
extern int GPU_cw(u32), func_800CF7E8(int);
extern void gpu_memset(u8 *, int, int);

int ResetGraph(int mode) {
    switch (mode & 7) {
    case 0:
    case 3:
        printf(gpuResetStateFormat, gpuDispatchTable, &gGpuInfo);
    case 5:
        gpu_memset((u8 *)&gGpuInfo, 0, 0x80);
        ResetCallback();
        GPU_cw((u32)gGpuDispatch & 0xffffff);
        gGpuInfo.version = func_800CF7E8(mode);
        gGpuInfo.unk1 = 1;
        gGpuInfo.w = gpuDefaultWidths[(*(volatile u8 *)&gGpuInfo.version) * 2];
        gGpuInfo.h = gpuDefaultHeights[(*(volatile u8 *)&gGpuInfo.version) * 2];
        gpu_memset((u8 *)&gGpuInfo.draw, -1, 0x5c);
        gpu_memset((u8 *)&gGpuInfo.disp, -1, 0x14);
        return gGpuInfo.version;
    default:
        if (gGpuInfo.level >= 2) GPU_printf(gpuResetGraphFormat, mode);
        return gGpuDispatch->reset(1);
    }
}

int SetGraphDebug(int level) {
    int old = gGpuInfo.level;
    gGpuInfo.level = level;
    if ((u8)level)
        GPU_printf(gpuGraphDebugFormat, gGpuInfo.level, gGpuInfo.version, gGpuInfo.reverse);
    return old;
}

#ifdef NON_MATCHING
/* Best observed instruction-sequence similarity: 95.24% (40/42 words;
 * GCC 2.7.2). The debug-level load is hoisted before the prologue rather
 * than using the saved queue-state address, adding one instruction. */
int SetGraphQueue(int mode) {
    u8 old = gGpuInfo.unk1;
    if (gGpuInfo.level >= 2)
        GPU_printf(gpuGraphQueueFormat, mode);
    if (mode != gGpuInfo.unk1) {
        gGpuDispatch->reset(1);
        gGpuInfo.unk1 = mode;
        DMACallback(2, 0);
    }
    return old;
}
#else
INCLUDE_ASM("asm/nonmatchings/syslib/libgpu/sys", SetGraphQueue);
#endif

int GetGraphDebug(void) { return gGpuInfo.level; }

DmaCallback DrawSyncCallback(DmaCallback cb) {
    DmaCallback old;
    if (gGpuInfo.level >= 2)
        GPU_printf(gpuDrawSyncCallbackFormat, cb);
    old = gGpuInfo.drawSyncCb;
    gGpuInfo.drawSyncCb = cb;
    return old;
}

extern void gpu_memset(u8 *, int, int);

void SetDispMask(int mask) {
    if (gGpuInfo.level >= 2)
        GPU_printf(gpuDisplayMaskFormat, mask);
    if (!mask)
        gpu_memset((u8 *)&gGpuInfo.disp, -1, 20);
    gGpuDispatch->ctl(mask ? 0x03000000 : 0x03000001);
}

int DrawSync(int mode) {
    if (gGpuInfo.level >= 2)
        GPU_printf(gpuDrawSyncFormat, mode);
    return gGpuDispatch->sync(mode);
}

void func_800CD4CC(const char *name, RECT *r) {
    switch (gGpuInfo.level) {
    case 1:
        if (r->w > gGpuInfo.w || r->w + r->x > gGpuInfo.w || r->y > gGpuInfo.h ||
            r->y + r->h > gGpuInfo.h || r->w <= 0 || r->x < 0 || r->y < 0 || r->h <= 0) {
            GPU_printf(gpuInvalidRectangleFormat, name);
            GPU_printf(gpuRectangleCoordinatesFormat, r->x, r->y, r->w, r->h);
        }
        break;
    case 2:
        GPU_printf(gpuOperationPrefixFormat, name);
        GPU_printf(gpuRectangleCoordinatesFormat, r->x, r->y, r->w, r->h);
        break;
    }
}

extern void func_800CD4CC(const char *, RECT *);

s32 ClearImage(RECT *rect, u8 r, u8 g, u8 b) {
    func_800CD4CC(gpuClearImageName, rect);
    return gGpuDispatch->addque2((s32 (*)(u32, u32))gGpuDispatch->clr,
                                 (u32)rect, 8, (b << 16) | (g << 8) | r);
}

s32 ClearImage2(RECT *rect, u8 r, u8 g, u8 b) {
    func_800CD4CC(gpuClearImage2Name, rect);
    return gGpuDispatch->addque2((s32 (*)(u32, u32))gGpuDispatch->clr,
                                 (u32)rect, 8, 0x80000000 | (b << 16) | (g << 8) | r);
}

s32 LoadImage(RECT *rect, u32 *pixels) {
    func_800CD4CC(gpuLoadImageName, rect);
    return gGpuDispatch->addque2((s32 (*)(u32, u32))gGpuDispatch->dws,
                                 (u32)rect, 8, (u32)pixels);
}

s32 StoreImage(RECT *rect, u32 *pixels) {
    func_800CD4CC(gpuStoreImageName, rect);
    return gGpuDispatch->addque2((s32 (*)(u32, u32))gGpuDispatch->drs,
                                 (u32)rect, 8, (u32)pixels);
}

s32 MoveImage(RECT *rect, int x, int y) {
    u32 source;
    func_800CD4CC(gpuMoveImageName, rect);
    if (rect->w == 0 || rect->h == 0)
        return -1;
    source = *(u32 *)&rect->x;
    sGpuMovePacket.source = source;
    sGpuMovePacket.destination = ((u32)y << 16) | (x & 0xffff);
    sGpuMovePacket.size = *(u32 *)&rect->w;
    return gGpuDispatch->addque2((s32 (*)(u32, u32))gGpuDispatch->cwc,
                                 (u32)&sGpuMovePacket, sizeof(sGpuMovePacket), 0);
}

#ifdef NON_MATCHING
extern u32 gpuOTagTerminatorPacket, gpuOTagLinkPacket;

/* Best observed instruction-sequence similarity: 82.35% (42/51 words). */
u32 *ClearOTag(u32 *ot, int count) {
    if (gGpuInfo.level >= 2)
        GPU_printf(gpuClearOTagFormat, ot, count);
    if (--count) do {
        P_TAG *tag = (P_TAG *)ot;
        tag->len = 0;
        tag->addr = (u32)(ot + 1) & 0xffffff;
        ot++;
    } while (--count);
    gpuOTagLinkPacket = ((u32)&gpuOTagTerminatorPacket & 0xffffff) | 0x04000000;
    *ot = (u32)&gpuOTagLinkPacket & 0xffffff;
    return ot;
}

/* Best observed instruction-sequence similarity: 79.55% (35/44 words). */
u32 *ClearOTagR(u32 *ot, int count) {
    if (gGpuInfo.level >= 2)
        GPU_printf(gpuClearOTagReverseFormat, ot, count);
    gGpuDispatch->otc(ot, count);
    gpuOTagLinkPacket = ((u32)&gpuOTagTerminatorPacket & 0xffffff) | 0x04000000;
    *ot = (u32)&gpuOTagLinkPacket & 0xffffff;
    return ot;
}
#else
INCLUDE_ASM("asm/nonmatchings/syslib/libgpu/sys", ClearOTag);
INCLUDE_ASM("asm/nonmatchings/syslib/libgpu/sys", ClearOTagR);
#endif

void DrawPrim(void* p) {
    int len = getlen(p);
    gGpuDispatch->sync(0);
    gGpuDispatch->cwb((u32*)&((P_TAG*)p)->r0, len);
}

s32 DrawOTag(u32 *ot) {
    if (gGpuInfo.level >= 2)
        GPU_printf(gpuDrawOTagFormat, ot);
    return gGpuDispatch->addque2((s32 (*)(u32, u32))gGpuDispatch->cwc,
                                 (u32)ot, 0, 0);
}

extern void func_800CE584(DR_ENV *, DRAWENV *);

DRAWENV *PutDrawEnv(DRAWENV *env) {
    DR_ENV *packet;
    if (gGpuInfo.level >= 2)
        GPU_printf(gpuPutDrawEnvFormat, env);
    packet = &env->dr_env;
    func_800CE584(packet, env);
    packet->tag |= 0xffffff;
    gGpuDispatch->addque2((s32 (*)(u32, u32))gGpuDispatch->cwc,
                          (u32)packet, 64, 0);
    memcpy(&gGpuInfo.draw, env, sizeof(DRAWENV));
    return env;
}

void DrawOTagEnv(u32 *ot, DRAWENV *env) {
    DR_ENV *packet;
    if (gGpuInfo.level >= 2)
        GPU_printf(gpuDrawOTagEnvFormat, ot, env);
    packet = &env->dr_env;
    func_800CE584(packet, env);
    packet->tag = (packet->tag & 0xff000000) | ((u32)ot & 0xffffff);
    gGpuDispatch->addque2((s32 (*)(u32, u32))gGpuDispatch->cwc,
                          (u32)packet, 64, 0);
    memcpy(&gGpuInfo.draw, env, sizeof(DRAWENV));
}

DRAWENV* GetDrawEnv(DRAWENV* env) {
    memcpy(env, &gGpuInfo.draw, sizeof(DRAWENV));
    return env;
}

INCLUDE_ASM("asm/nonmatchings/syslib/libgpu/sys", PutDispEnv);

DISPENV* GetDispEnv(DISPENV* env) {
    memcpy(env, &gGpuInfo.disp, sizeof(DISPENV));
    return env;
}

int GetODE(void) { return gGpuDispatch->status() >> 0x1F; }

void SetTexWindow(DR_TWIN* p, RECT* tw) {
    setlen(p, 2);
    p->code[0] = gpuGetTextureWindow(tw);
    p->code[1] = 0;
}

void SetDrawArea(DR_AREA* p, RECT* r) {
    setlen(p, 2);
    p->code[0] = gpuGetClipStart(r->x, r->y);
    p->code[1] = gpuGetClipEnd(r->x + r->w - 1, r->y + r->h - 1);
}

void SetDrawOffset(DR_OFFSET* p, u16* ofs) {
    setlen(p, 2);
    p->code[0] = gpuGetOffset(ofs[0], ofs[1]);
    p->code[1] = 0;
}

void SetPriority(DR_PRIO* p, int pbc, int pbw) {
    u32 data;
    setlen(p, 2);
    data = 0xE6000000;
    if (pbc) {
        data = 0xE6000002;
    }
    p->code[0] = data | (pbw != 0);
    p->code[1] = 0;
}

void SetDrawStp(DR_STP *p, s32 stp) {
    setlen(p, 2);
    if (stp) {
        p->code[0] = 0xe6000001;
    } else {
        p->code[0] = 0xe6000000;
    }
    p->code[1] = 0;
}

void SetDrawMode(DR_MODE* p, int dfe, int dtd, int tpage, RECT* tw) {
    setlen(p, 2);
    p->code[0] = gpuGetMode(dfe, dtd, tpage);
    p->code[1] = gpuGetTextureWindow(tw);
}

#ifdef NON_MATCHING
/* Best observed instruction-sequence similarity: 65.62% (84/128 words;
 * GCC 2.8.0, -mno-split-addresses). */
void SetDrawEnv(DR_ENV *packet, DRAWENV *env) {
    int n = 7;
    RECT rect;
    packet->code[0] = gpuGetClipStart(env->clip.x, env->clip.y);
    packet->code[1] = gpuGetClipEnd(env->clip.x + env->clip.w - 1, env->clip.y + env->clip.h - 1);
    packet->code[2] = gpuGetOffset(env->ofs[0], env->ofs[1]);
    packet->code[3] = gpuGetMode(env->dfe, env->dtd, env->tpage);
    packet->code[4] = gpuGetTextureWindow(&env->tw);
    packet->code[5] = 0xe6000000;
    if (env->isbg) {
        rect.x = env->clip.x; rect.y = env->clip.y; rect.w = env->clip.w; rect.h = env->clip.h;
        if (rect.w < 0) rect.w = 0;
        else if (rect.w > gGpuInfo.w - 1) rect.w = gGpuInfo.w - 1;
        if (rect.h < 0) rect.h = 0;
        else if (rect.h > gGpuInfo.h - 1) rect.h = gGpuInfo.h - 1;
        rect.x -= env->ofs[0]; rect.y -= env->ofs[1];
        ((u32 *)packet)[n++] = 0x60000000 | (env->b0 << 16) | (env->g0 << 8) | env->r0;
        ((u32 *)packet)[n++] = *(u32 *)&rect.x;
        ((u32 *)packet)[n++] = *(u32 *)&rect.w;
    }
    setlen(packet, n - 1);
}
#else
INCLUDE_ASM("asm/nonmatchings/syslib/libgpu/sys", SetDrawEnv);
#endif

INCLUDE_ASM("asm/nonmatchings/syslib/libgpu/sys", func_800CE584);

u32 gpuGetMode(int dfe, int dtd, u16 tpage) {
    return (dtd ? 0xE1000200 : 0xE1000000) | (dfe ? 0x400 : 0) | (tpage & 0x9FF);
}

u32 gpuGetClipStart(s16 x, s16 y) {
    x = x < 0 ? 0 : x > gGpuInfo.w - 1 ? gGpuInfo.w - 1 : x;
    y = y < 0 ? 0 : y > gGpuInfo.h - 1 ? gGpuInfo.h - 1 : y;
    return 0xe3000000 | ((y & 0x3ff) << 10) | (x & 0x3ff);
}

u32 gpuGetClipEnd(s16 x, s16 y) {
    x = x < 0 ? 0 : x > gGpuInfo.w - 1 ? gGpuInfo.w - 1 : x;
    y = y < 0 ? 0 : y > gGpuInfo.h - 1 ? gGpuInfo.h - 1 : y;
    return 0xe4000000 | ((y & 0x3ff) << 10) | (x & 0x3ff);
}

u32 gpuGetOffset(s16 x, s16 y) { return 0xE5000000 | ((y & 0x7FF) << 11) | (x & 0x7FF); }

u32 gpuGetTextureWindow(RECT* rect) {
    u32 code[4];
    if (rect) {
        code[0] = (rect->x & 0xFF) >> 3;
        code[2] = (-rect->w & 0xFF) >> 3;
        code[1] = (rect->y & 0xFF) >> 3;
        code[3] = (-rect->h & 0xFF) >> 3;
        return 0xE2000000 | (code[1] << 15) | (code[0] << 10) | (code[3] << 5) |
               code[2];
    }
    return 0;
}

u32 _status(void) { return *GPU_STATUS; }

int _otc(u32 *ot, int n) {
    *DPCR |= 0x08000000;
    *DMA6_CHCR = 0;
    *DMA6_MADR = (u32)(ot - 1 + n);
    *DMA6_BCR = n;
    *DMA6_CHCR = 0x11000002;
    set_alarm();
    while (*DMA6_CHCR & 0x01000000) {
        if (get_alarm()) return -1;
    }
    return n;
}

#ifdef NON_MATCHING
extern u32 gpuClearPacket[], gpuRestoreDrawAreaPacket[];
extern int _param(int);
extern void _cwc(u32 *);
/* Best observed instruction-sequence similarity: 86.43% (121/140 words;
 * GCC 2.8.0, -mno-split-addresses). */
int func_800CEAD8(RECT *r, u32 color) {
    if (r->w < 0) r->w = 0;
    else if (r->w > gGpuInfo.w - 1) r->w = gGpuInfo.w - 1;
    if (r->h < 0) r->h = 0;
    else if (r->h > gGpuInfo.h - 1) r->h = gGpuInfo.h - 1;
    if ((r->x & 63) || (r->w & 63)) {
        gpuClearPacket[0] = ((u32)gpuRestoreDrawAreaPacket & 0xffffff) | 0x08000000;
        gpuClearPacket[1] = 0xe3000000;
        gpuClearPacket[2] = 0xe4ffffff;
        gpuClearPacket[3] = 0xe5000000;
        gpuClearPacket[4] = 0xe6000000;
        gpuClearPacket[5] = 0xe1000000 | (*GPU_STATUS & 0x7ff) | ((color >> 31) << 10);
        gpuClearPacket[6] = 0x60000000 | (color & 0xffffff);
        gpuClearPacket[7] = ((u32 *)r)[0];
        gpuClearPacket[8] = ((u32 *)r)[1];
        gpuRestoreDrawAreaPacket[0] = 0x03ffffff;
        gpuRestoreDrawAreaPacket[1] = _param(3) | 0xe3000000;
        gpuRestoreDrawAreaPacket[2] = _param(4) | 0xe4000000;
        gpuRestoreDrawAreaPacket[3] = _param(5) | 0xe5000000;
    } else {
        gpuClearPacket[0] = 0x05ffffff;
        gpuClearPacket[1] = 0xe6000000;
        gpuClearPacket[2] = 0xe1000000 | (*GPU_STATUS & 0x7ff) | ((color >> 31) << 10);
        gpuClearPacket[3] = 0x02000000 | (color & 0xffffff);
        gpuClearPacket[4] = ((u32 *)r)[0];
        gpuClearPacket[5] = ((u32 *)r)[1];
    }
    _cwc(gpuClearPacket);
    return 0;
}
#else
INCLUDE_ASM("asm/nonmatchings/syslib/libgpu/sys", func_800CEAD8);
#endif

#ifdef NON_MATCHING
extern void set_alarm(void);
extern int get_alarm(void);
/* Best observed instruction-sequence similarity: 57.34% (82/143 words;
 * GCC 2.8.0, -mno-split-addresses). */
int func_800CED08(RECT *r, u32 *data) {
    int words, blocks, rest;
    set_alarm();
    if (r->w < 0) r->w = 0;
    else if (r->w > gGpuInfo.w) r->w = gGpuInfo.w;
    if (r->h < 0) r->h = 0;
    else if (r->h > gGpuInfo.h) r->h = gGpuInfo.h;
    words = (r->w * r->h + 1) / 2;
    blocks = words / 16;
    rest = words - blocks * 16;
    if (words <= 0) return -1;
    while (!(*GPU_STATUS & 0x04000000)) if (get_alarm()) return -1;
    *GPU_STATUS = 0x04000000;
    *GPU_DATA = 0x01000000;
    *GPU_DATA = 0xa0000000;
    *GPU_DATA = ((u32 *)r)[0];
    *GPU_DATA = ((u32 *)r)[1];
    while (rest--) *GPU_DATA = *data++;
    if (blocks) {
        *GPU_STATUS = 0x04000002;
        *DMA2_MADR = (u32)data;
        *DMA2_BCR = (blocks << 16) | 0x10;
        *DMA2_CHCR = 0x01000201;
    }
    return 0;
}
#else
INCLUDE_ASM("asm/nonmatchings/syslib/libgpu/sys", func_800CED08);
#endif

INCLUDE_ASM("asm/nonmatchings/syslib/libgpu/sys", func_800CEF44);

void func_800CF1C4(u32 value) {
    *GPU_STATUS = value;
    gpuControlCache[value >> 24] = value;
}

s32 _getctl(s32 index) {
    return gpuControlCache[index];
}

int _cwb(u32 *data, s32 n) {
    int i = n - 1;

    *GPU_STATUS = 0x04000000;
    if (n) {
        do {
            *GPU_DATA = *data++;
        } while (--i != -1);
    }
    return 0;
}

void _cwc(u32 *packets) {
    *GPU_STATUS = 0x04000002;
    *DMA2_MADR = (u32)packets;
    *DMA2_BCR = 0;
    *DMA2_CHCR = 0x01000401;
}

int _param(int x) {
    *GPU_STATUS = x | 0x10000000;
    return *GPU_DATA & 0xFFFFFF;
}

extern int _addque2(int (*exec)(u32, u32), u32 p1, int len, u32 p2);

int _addque(int (*exec)(u32, u32), u32 p1, u32 p2) {
    return _addque2(exec, p1, 0, p2);
}

#ifdef NON_MATCHING
typedef struct { int (*execute)(u32, u32); u32 first, second; u32 buffer[21]; } GpuAddQueueEntry;
extern volatile s32 _qin, _qout;
extern s32 gpuAddQueueSavedMask;
extern u8 gpuCommandQueue[];
extern void set_alarm(void);
extern int get_alarm(void), _exeque(void);
/* Best observed instruction-sequence similarity: 65.70% (113/172 words;
 * GCC 2.7.2). */
int _addque2(int (*execute)(u32, u32), u32 data, int size, u32 second) {
    GpuAddQueueEntry *queue = (GpuAddQueueEntry *)gpuCommandQueue;
    int i;
    set_alarm();
    while (((_qin + 1) & 63) == _qout) {
        if (get_alarm()) return -1;
        _exeque();
    }
    gpuAddQueueSavedMask = SetIntrMask(0);
    *(u32 *)gGpuInfo.unk8 = 1;
    if (!gGpuInfo.unk1 || (_qin == _qout && !(*DMA2_CHCR & 0x01000000) && !gGpuInfo.drawSyncCb)) {
        while (!(*GPU_STATUS & 0x04000000)) {}
        execute(data, second);
        SetIntrMask(gpuAddQueueSavedMask);
        return 0;
    }
    DMACallback(2, (DmaCallback)_exeque);
    if (size) {
        for (i = 0; i < size / 4; i++) queue[_qin].buffer[i] = ((u32 *)data)[i];
        queue[_qin].first = (u32)queue[_qin].buffer;
    } else queue[_qin].first = data;
    queue[_qin].second = second;
    queue[_qin].execute = execute;
    _qin = (_qin + 1) & 63;
    SetIntrMask(gpuAddQueueSavedMask);
    _exeque();
    return (_qin - _qout) & 63;
}
#else
INCLUDE_ASM("asm/nonmatchings/syslib/libgpu/sys", _addque2);
#endif

#ifdef NON_MATCHING
typedef struct { int (*execute)(u32, u32); u32 first, second; u8 padding[84]; } GpuQueueEntry;
extern volatile s32 _qin, _qout;
extern s32 gpuExecuteQueueSavedMask;
extern u8 gpuCommandQueue[];
/* Best observed instruction-sequence similarity: 75.00% (114/152 words;
 * GCC 2.7.2). */
int _exeque(void) {
    GpuQueueEntry *queue = (GpuQueueEntry *)gpuCommandQueue;
    if (*DMA2_CHCR & 0x01000000) return 1;
    gpuExecuteQueueSavedMask = SetIntrMask(0);
    while (_qin != _qout && !(*DMA2_CHCR & 0x01000000)) {
        if (((_qout + 1) & 63) == _qin && !gGpuInfo.drawSyncCb) DMACallback(2, 0);
        while (!(*GPU_STATUS & 0x04000000)) {}
        queue[_qout].execute(queue[_qout].first, queue[_qout].second);
        _qout = (_qout + 1) & 63;
    }
    SetIntrMask(gpuExecuteQueueSavedMask);
    if (_qin == _qout && !(*DMA2_CHCR & 0x01000000) &&
        *(u32 *)gGpuInfo.unk8 && gGpuInfo.drawSyncCb) {
        *(u32 *)gGpuInfo.unk8 = 0;
        gGpuInfo.drawSyncCb();
    }
    return (_qin - _qout) & 63;
}
#else
INCLUDE_ASM("asm/nonmatchings/syslib/libgpu/sys", _exeque);
#endif

#ifdef NON_MATCHING
extern volatile s32 _qin, _qout;
extern s32 gpuTimeoutSavedMask;
extern u8 gpuCommandQueue[];
extern int _version(int);
/* Best observed instruction-sequence similarity: 86.21% (75/87 words). */
int func_800CF7E8(int mode) {
    gpuTimeoutSavedMask = SetIntrMask(0);
    _qout = 0;
    _qin = _qout;
    switch (mode & 7) {
    case 0:
        *DMA2_CHCR = 0x401;
        *DPCR |= 0x800;
        *GPU_STATUS = 0;
        gpu_memset(gpuControlCache, 0, 0x100);
        gpu_memset(gpuCommandQueue, 0, 0x1800);
        break;
    case 1:
    case 3:
    case 5:
        *DMA2_CHCR = 0x401;
        *DPCR |= 0x800;
        *GPU_STATUS = 0x02000000;
        *GPU_STATUS = 0x01000000;
        break;
    }
    SetIntrMask(gpuTimeoutSavedMask);
    if ((mode & 7) == 0) return _version(mode);
    return 0;
}
#else
INCLUDE_ASM("asm/nonmatchings/syslib/libgpu/sys", func_800CF7E8);
#endif

extern volatile s32 _qin, _qout;

int func_800CF938(int mode) {
    int count;
    if (!mode) {
        set_alarm();
        while (_qin != _qout) {
            _exeque();
            if (get_alarm()) return -1;
        }
        while ((*DMA2_CHCR & 0x01000000) || !(*GPU_STATUS & 0x04000000))
            if (get_alarm()) return -1;
        return 0;
    }
    count = (_qin - _qout) & 0x3f;
    if (count) _exeque();
    if ((*DMA2_CHCR & 0x01000000) || !(*GPU_STATUS & 0x04000000)) {
        if (!count) return 1;
    }
    return count;
}

void set_alarm(void) {
    gpuAlarmDeadline = VSync(-1) + 0xF0;
    gpuAlarmPollCount = 0;
}

#ifdef NON_MATCHING
extern s32 gpuTimeoutSavedMask;
extern int printf(const char *, ...);

/* Best observed instruction-sequence similarity: 90.12% (73/81 words). */
int get_alarm(void) {
    if (VSync(-1) > gpuAlarmDeadline || gpuAlarmPollCount++ > 0xF0000) {
        *GPU_STATUS;
        printf(gpuQueueTimeoutFormat, (_qin - _qout) & 0x3F,
               *GPU_STATUS, *DMA2_CHCR, *DMA2_MADR);
        gpuTimeoutSavedMask = SetIntrMask(0);
        _qin = _qout = 0;
        *DMA2_CHCR = 0x401;
        *DPCR |= 0x800;
        *GPU_STATUS = 0x02000000;
        *GPU_STATUS = 0x01000000;
        SetIntrMask(gpuTimeoutSavedMask);
        return -1;
    }
    return 0;
}
#else
INCLUDE_ASM("asm/nonmatchings/syslib/libgpu/sys", get_alarm);
#endif

int _version(int mode) {
    *GPU_STATUS = 0x10000007;
    if ((*GPU_DATA & 0xFFFFFF) != 2) {
        *GPU_DATA = 0xE1001000 | (*GPU_STATUS & 0x3FFF);
        *GPU_DATA;
        return 0;
    }
    if (!(mode & 8)) return 1;
    *GPU_STATUS = 0x09000001;
    return 2;
}

extern DmaCallback gpu_install_dma_callback(void);

s32 LoadImage2(RECT *rect, u32 *pixels) {
    func_800CD4CC(gpuLoadImage2Name, rect);
    gpuAlarmDeadline = VSync(-1) + 240;
    gpuAlarmPollCount = 0;
    while ((*DMA2_CHCR & 0x01000000) || !(*GPU_STATUS & 0x04000000))
        if (get_alarm())
            return -1;
    DMACallback(2, (DmaCallback)gpu_install_dma_callback);
    gGpuDispatch->dws(rect, pixels);
    return 0;
}

s32 StoreImage2(RECT *rect, u32 *pixels) {
    func_800CD4CC(gpuStoreImageName, rect);
    gpuAlarmDeadline = VSync(-1) + 240;
    gpuAlarmPollCount = 0;
    while ((*DMA2_CHCR & 0x01000000) || !(*GPU_STATUS & 0x04000000))
        if (get_alarm())
            return -1;
    DMACallback(2, (DmaCallback)gpu_install_dma_callback);
    gGpuDispatch->drs(rect, pixels);
    return 0;
}

s32 MoveImage2(RECT *rect, int x, int y) {
    u32 source;
    func_800CD4CC(gpuMoveImageName, rect);
    gpuAlarmDeadline = VSync(-1) + 240;
    gpuAlarmPollCount = 0;
    while ((*DMA2_CHCR & 0x01000000) || !(*GPU_STATUS & 0x04000000))
        if (get_alarm())
            return -1;
    DMACallback(2, (DmaCallback)gpu_install_dma_callback);
    if (rect->w == 0 || rect->h == 0)
        return -1;
    source = *(u32 *)&rect->x;
    sGpuMovePacket.source = source;
    sGpuMovePacket.destination = ((u32)y << 16) | (x & 0xffff);
    sGpuMovePacket.size = *(u32 *)&rect->w;
    gGpuDispatch->cwc((u32 *)&sGpuMovePacket);
    return 0;
}

s32 DrawOTag2(u32 *ot) {
    if (gGpuInfo.level >= 2)
        GPU_printf(gpuDrawOTagFormat, ot);
    gpuAlarmDeadline = VSync(-1) + 240;
    gpuAlarmPollCount = 0;
    while ((*DMA2_CHCR & 0x01000000) || !(*GPU_STATUS & 0x04000000))
        if (get_alarm())
            return -1;
    DMACallback(2, (DmaCallback)gpu_install_dma_callback);
    gGpuDispatch->cwc(ot);
    return 0;
}

DmaCallback gpu_install_dma_callback(void) {
    return DMACallback(2, (DmaCallback)_exeque);
}

void gpu_memset(u8 *ptr, int value, int n) {
    int i = n - 1;

    if (n) {
        do {
            *ptr++ = value;
        } while (--i != -1);
    }
}
