#include "common.h"

#ifdef NON_MATCHING
#include "libgs_private.h"
typedef struct {
    s32 attribute;
    s16 x, y;
    u16 w, h, tpage;
    u8 u, v;
    s16 cx, cy;
    u8 r, g, b, pad;
} FastSprite;
extern u32 *D_8013F8E8;
extern s16 D_8013F754, D_8013F756;
/* Best observed instruction-sequence similarity: 44.79% (43/96 words;
 * target includes padding). */
void GsSortFastSprite(FastSprite *s, GsOT *ot, u16 depth) {
    u32 *p, *tag, addr, attr;
    if (s->attribute < 0 || !s->w || !s->h) return;
    attr = s->attribute;
    p = D_8013F8E8;
    addr = (u32)p & 0xffffff;
    p[1] = 0xe1000200 | ((attr >> 17) & 0x180) | ((attr >> 23) & 0x60) | (s->tpage & 0x1f);
    p[3] = (u16)(s->x + D_8013F754) | ((s->y + D_8013F756) << 16);
    p[2] = 0x64000000 | ((attr >> 5) & 0x02000000) | ((attr << 18) & 0x01000000) |
           (s->b << 16) | (s->g << 8) | s->r;
    p[4] = s->u | (s->v << 8) | (s->cy << 22) | ((s->cx << 12) & 0x3f0000);
    p[5] = s->w | (s->h << 16);
    tag = ot->org + depth - ot->offset;
    p[0] = *tag + 0x05000000;
    *tag = addr;
    D_8013F8E8 = p + 6;
}
#else
INCLUDE_ASM("asm/nonmatchings/syslib/libgs/2d_sp1", GsSortFastSprite);
#endif
