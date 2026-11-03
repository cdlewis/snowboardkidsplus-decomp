#include "common.h"
#include "libgs_private.h"

extern void func_800BD3FC(u16, u16, u16, u16, u16);
extern void gte_init(void), func_800BD580(u16, u16);
extern void GsSetDrawBuffClip(void), GsSetDrawBuffOffset(void);
extern s16 PSDIDX;

void GsInitGraph(u16 w, u16 h, u16 a, u16 b, u16 c) {
    func_800BD3FC(w, h, a, b, c);
    gte_init();
    PSDIDX = 0;
    func_800BD580(w, h);
    GsSetDrawBuffClip();
    GsSetDrawBuffOffset();
}

#ifdef NON_MATCHING
extern int ResetGraph(int), GetVideoMode(void);
extern void PutDrawEnv(DRAWENV *), PutDispEnv(DISPENV *);
/* Best observed instruction-sequence similarity: 70.42% (50/71 words;
 * GCC 2.8.0, -mno-split-addresses). */
void func_800BD3FC(u16 w, u16 h, u16 flags, u16 a, u16 b) {
    int reset = 0;
    if (((flags >> 4) & 3) == 3) reset = 3;
    ResetGraph(reset);
    GsDRAWENV.ofs[1] = 0;
    GsDRAWENV.ofs[0] = 0;
    GsDRAWENV.tw.y = 0;
    GsDRAWENV.tw.x = 0;
    GsDRAWENV.tw.h = 0;
    GsDRAWENV.tw.w = 0;
    GsDRAWENV.tpage = 0;
    GsDRAWENV.dtd = a;
    GsDRAWENV.dfe = 0;
    GsDRAWENV.isbg = 0;
    PutDrawEnv(&GsDRAWENV);
    GsDISPENV.disp.x = 0;
    GsDISPENV.disp.y = 0;
    GsDISPENV.disp.w = w;
    GsDISPENV.disp.h = h;
    GsDISPENV.screen.x = 0;
    GsDISPENV.screen.y = 0;
    GsDISPENV.screen.w = 0;
    GsDISPENV.screen.h = 0;
    if (GetVideoMode() == 1) {
        GsDISPENV.screen.y = 24;
        GsDISPENV.pad0 = 1;
    }
    GsDISPENV.isinter = flags & 1;
    PSDGPU = flags & 4;
    GsDISPENV.isrgb24 = b;
    PutDispEnv(&GsDISPENV);
}
#else
INCLUDE_ASM("asm/nonmatchings/syslib/libgs/gs_001", func_800BD3FC);
#endif

void GsInitGraph2(u16 w, u16 h, u16 flags, u16 a, u16 b) {
    GsDRAWENV.tpage = 0;
    GsDRAWENV.dtd = a;
    GsDRAWENV.dfe = 0;
    GsDRAWENV.isbg = 0;
    GsDISPENV.disp.w = w;
    GsDISPENV.disp.h = h;
    GsDISPENV.isinter = flags & 1;
    PSDGPU = flags & 4;
    GsDISPENV.isrgb24 = b;
    func_800BD580(w, h);
}

INCLUDE_ASM("asm/nonmatchings/syslib/libgs/gs_001", func_800BD580);

#ifdef NON_MATCHING
typedef struct { u32 tag; u8 r, g, b, code; s16 x, y, w, h; } ClearTile;
extern ClearTile gsClearTiles[];
extern s16 PSDOFSX[], PSDOFSY[];
extern int HWD0, VWD0;
/* Instruction-sequence similarity: 49.38% (40/81 words; GCC 2.7.2).
 * Best experiment: 55.56% (45/81), but its volatile global declaration
 * changed the matching GsInitGraph code. */
void GsSortClear(u8 r, u8 g, u8 b, GsOT *ot) {
    int i;
    gsClearTiles[*(volatile s16 *)&PSDIDX].r = r;
    gsClearTiles[*(volatile s16 *)&PSDIDX].g = g;
    gsClearTiles[*(volatile s16 *)&PSDIDX].b = b;
    i = *(volatile s16 *)&PSDIDX;
    gsClearTiles[i].x = PSDOFSX[i];
    gsClearTiles[i].h = VWD0;
    gsClearTiles[i].y = PSDOFSY[i];
    if (GsDISPENV.isrgb24) gsClearTiles[i].w = HWD0 * 3 / 2;
    else gsClearTiles[i].w = HWD0;
    AddPrim(ot->tag, &gsClearTiles[*(volatile s16 *)&PSDIDX]);
}
#else
INCLUDE_ASM("asm/nonmatchings/syslib/libgs/gs_001", GsSortClear);
#endif
