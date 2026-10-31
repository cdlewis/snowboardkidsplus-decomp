#include "common.h"
#include "libgpu_private.h"

/* C routines adapted from PsyZ decomp; see THIRD_PARTY.md. */

INCLUDE_ASM("asm/nonmatchings/syslib/libgpu/sys", ResetGraph);

INCLUDE_ASM("asm/nonmatchings/syslib/libgpu/sys", SetGraphDebug);

INCLUDE_ASM("asm/nonmatchings/syslib/libgpu/sys", SetGraphQueue);

int GetGraphDebug(void) { return gGpuInfo.level; }

INCLUDE_ASM("asm/nonmatchings/syslib/libgpu/sys", DrawSyncCallback);

INCLUDE_ASM("asm/nonmatchings/syslib/libgpu/sys", SetDispMask);

INCLUDE_ASM("asm/nonmatchings/syslib/libgpu/sys", DrawSync);

INCLUDE_ASM("asm/nonmatchings/syslib/libgpu/sys", func_800CD4CC);

INCLUDE_ASM("asm/nonmatchings/syslib/libgpu/sys", ClearImage);

INCLUDE_ASM("asm/nonmatchings/syslib/libgpu/sys", ClearImage2);

INCLUDE_ASM("asm/nonmatchings/syslib/libgpu/sys", LoadImage);

INCLUDE_ASM("asm/nonmatchings/syslib/libgpu/sys", StoreImage);

INCLUDE_ASM("asm/nonmatchings/syslib/libgpu/sys", MoveImage);

INCLUDE_ASM("asm/nonmatchings/syslib/libgpu/sys", ClearOTag);

INCLUDE_ASM("asm/nonmatchings/syslib/libgpu/sys", ClearOTagR);

void DrawPrim(void* p) {
    int len = getlen(p);
    gGpuDispatch->sync(0);
    gGpuDispatch->cwb((u32*)&((P_TAG*)p)->r0, len);
}

INCLUDE_ASM("asm/nonmatchings/syslib/libgpu/sys", DrawOTag);

INCLUDE_ASM("asm/nonmatchings/syslib/libgpu/sys", PutDrawEnv);

INCLUDE_ASM("asm/nonmatchings/syslib/libgpu/sys", DrawOTagEnv);

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

INCLUDE_ASM("asm/nonmatchings/syslib/libgpu/sys", SetDrawStp);

void SetDrawMode(DR_MODE* p, int dfe, int dtd, int tpage, RECT* tw) {
    setlen(p, 2);
    p->code[0] = gpuGetMode(dfe, dtd, tpage);
    p->code[1] = gpuGetTextureWindow(tw);
}

INCLUDE_ASM("asm/nonmatchings/syslib/libgpu/sys", SetDrawEnv);

INCLUDE_ASM("asm/nonmatchings/syslib/libgpu/sys", func_800CE584);

u32 gpuGetMode(int dfe, int dtd, u16 tpage) {
    return (dtd ? 0xE1000200 : 0xE1000000) | (dfe ? 0x400 : 0) | (tpage & 0x9FF);
}

INCLUDE_ASM("asm/nonmatchings/syslib/libgpu/sys", gpuGetClipStart);

INCLUDE_ASM("asm/nonmatchings/syslib/libgpu/sys", gpuGetClipEnd);

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

INCLUDE_ASM("asm/nonmatchings/syslib/libgpu/sys", func_800CE9E0);

INCLUDE_ASM("asm/nonmatchings/syslib/libgpu/sys", func_800CE9F8);

INCLUDE_ASM("asm/nonmatchings/syslib/libgpu/sys", func_800CEAD8);

INCLUDE_ASM("asm/nonmatchings/syslib/libgpu/sys", func_800CED08);

INCLUDE_ASM("asm/nonmatchings/syslib/libgpu/sys", func_800CEF44);

INCLUDE_ASM("asm/nonmatchings/syslib/libgpu/sys", func_800CF1C4);

INCLUDE_ASM("asm/nonmatchings/syslib/libgpu/sys", func_800CF1E8);

INCLUDE_ASM("asm/nonmatchings/syslib/libgpu/sys", func_800CF1FC);

INCLUDE_ASM("asm/nonmatchings/syslib/libgpu/sys", func_800CF23C);

INCLUDE_ASM("asm/nonmatchings/syslib/libgpu/sys", func_800CF284);

INCLUDE_ASM("asm/nonmatchings/syslib/libgpu/sys", func_800CF2B4);

INCLUDE_ASM("asm/nonmatchings/syslib/libgpu/sys", func_800CF2D8);

INCLUDE_ASM("asm/nonmatchings/syslib/libgpu/sys", func_800CF588);

INCLUDE_ASM("asm/nonmatchings/syslib/libgpu/sys", func_800CF7E8);

INCLUDE_ASM("asm/nonmatchings/syslib/libgpu/sys", func_800CF938);

INCLUDE_ASM("asm/nonmatchings/syslib/libgpu/sys", func_800CFA74);

INCLUDE_ASM("asm/nonmatchings/syslib/libgpu/sys", func_800CFAA8);

INCLUDE_ASM("asm/nonmatchings/syslib/libgpu/sys", func_800CFBEC);

INCLUDE_ASM("asm/nonmatchings/syslib/libgpu/sys", LoadImage2);

INCLUDE_ASM("asm/nonmatchings/syslib/libgpu/sys", StoreImage2);

INCLUDE_ASM("asm/nonmatchings/syslib/libgpu/sys", MoveImage2);

INCLUDE_ASM("asm/nonmatchings/syslib/libgpu/sys", DrawOTag2);

INCLUDE_ASM("asm/nonmatchings/syslib/libgpu/sys", func_800D00A4);

INCLUDE_ASM("asm/nonmatchings/syslib/libgpu/sys", func_800D00CC);
