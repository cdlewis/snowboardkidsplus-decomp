#include "common.h"

extern void func_800BD3FC(u16, u16, u16, u16, u16);
extern void gte_init(void), func_800BD580(u16, u16);
extern void GsSetDrawBuffClip(void), GsSetDrawBuffOffset(void);
extern s16 D_8013F764;

void GsInitGraph(u16 w, u16 h, u16 a, u16 b, u16 c) {
    func_800BD3FC(w, h, a, b, c);
    gte_init();
    D_8013F764 = 0;
    func_800BD580(w, h);
    GsSetDrawBuffClip();
    GsSetDrawBuffOffset();
}

#ifdef NON_MATCHING
#include "psx/libgpu.h"
extern DRAWENV D_8013F6E0;
extern DISPENV D_8013F740;
extern s16 D_8013F766;
extern int ResetGraph(int), GetVideoMode(void);
extern void PutDrawEnv(DRAWENV *), PutDispEnv(DISPENV *);
/* Best observed instruction-sequence similarity: 70.42% (50/71 words;
 * GCC 2.8.0, -mno-split-addresses). */
void func_800BD3FC(u16 w, u16 h, u16 flags, u16 a, u16 b) {
    int reset = 0;
    if (((flags >> 4) & 3) == 3) reset = 3;
    ResetGraph(reset);
    D_8013F6E0.ofs[1] = 0;
    D_8013F6E0.ofs[0] = 0;
    D_8013F6E0.tw.y = 0;
    D_8013F6E0.tw.x = 0;
    D_8013F6E0.tw.h = 0;
    D_8013F6E0.tw.w = 0;
    D_8013F6E0.tpage = 0;
    D_8013F6E0.dtd = a;
    D_8013F6E0.dfe = 0;
    D_8013F6E0.isbg = 0;
    PutDrawEnv(&D_8013F6E0);
    D_8013F740.disp.x = 0;
    D_8013F740.disp.y = 0;
    D_8013F740.disp.w = w;
    D_8013F740.disp.h = h;
    D_8013F740.screen.x = 0;
    D_8013F740.screen.y = 0;
    D_8013F740.screen.w = 0;
    D_8013F740.screen.h = 0;
    if (GetVideoMode() == 1) {
        D_8013F740.screen.y = 24;
        D_8013F740.pad0 = 1;
    }
    D_8013F740.isinter = flags & 1;
    D_8013F766 = flags & 4;
    D_8013F740.isrgb24 = b;
    PutDispEnv(&D_8013F740);
}
#else
INCLUDE_ASM("asm/nonmatchings/syslib/libgs/gs_001", func_800BD3FC);
#endif

struct GsInitState {
    u16 unk0;
    u8 unk2, unk3, unk4;
};
struct GsDisplayState {
    u16 width, height;
    u8 unk4[8];
    u8 unkC, unkD;
};
extern struct GsInitState D_8013F6F4;
extern struct GsDisplayState D_8013F744;
extern s16 D_8013F766;

void GsInitGraph2(u16 w, u16 h, u16 flags, u16 a, u16 b) {
    D_8013F6F4.unk0 = 0;
    D_8013F6F4.unk2 = a;
    D_8013F6F4.unk3 = 0;
    D_8013F6F4.unk4 = 0;
    D_8013F744.width = w;
    D_8013F744.height = h;
    D_8013F744.unkC = flags & 1;
    D_8013F766 = flags & 4;
    D_8013F744.unkD = b;
    func_800BD580(w, h);
}

INCLUDE_ASM("asm/nonmatchings/syslib/libgs/gs_001", func_800BD580);

#ifdef NON_MATCHING
#include "psx/libgpu.h"
#include "libgs_private.h"
typedef struct { u32 tag; u8 r, g, b, code; s16 x, y, w, h; } ClearTile;
extern ClearTile D_8013F6A8[];
extern s16 D_8013F6C8[], D_8013F6CC[];
extern int D_8013F7E8, D_8013F7EC;
extern u8 D_8013F751;
/* Instruction-sequence similarity: 49.38% (40/81 words; GCC 2.7.2).
 * Best experiment: 55.56% (45/81), but its volatile global declaration
 * changed the matching GsInitGraph code. */
void GsSortClear(u8 r, u8 g, u8 b, GsOT *ot) {
    int i;
    D_8013F6A8[*(volatile s16 *)&D_8013F764].r = r;
    D_8013F6A8[*(volatile s16 *)&D_8013F764].g = g;
    D_8013F6A8[*(volatile s16 *)&D_8013F764].b = b;
    i = *(volatile s16 *)&D_8013F764;
    D_8013F6A8[i].x = D_8013F6C8[i];
    D_8013F6A8[i].h = D_8013F7EC;
    D_8013F6A8[i].y = D_8013F6CC[i];
    if (D_8013F751) D_8013F6A8[i].w = D_8013F7E8 * 3 / 2;
    else D_8013F6A8[i].w = D_8013F7E8;
    AddPrim(ot->tag, &D_8013F6A8[*(volatile s16 *)&D_8013F764]);
}
#else
INCLUDE_ASM("asm/nonmatchings/syslib/libgs/gs_001", GsSortClear);
#endif
