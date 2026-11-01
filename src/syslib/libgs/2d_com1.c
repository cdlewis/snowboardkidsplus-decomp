#include "common.h"

#ifdef NON_MATCHING
#include "libgs_private.h"
extern char D_80012A80[];
extern int printf(const char *, ...);
/* Best observed instruction-sequence similarity: 72.50% (29/40 words;
 * target includes padding). */
u32 *_make_packet(u32 *packet, GsOT *ot, u16 depth, u32 size) {
    int index = depth - ot->offset;
    u32 *tag;
    if (index < 0)
        printf(D_80012A80);
    tag = &ot->org[index];
    *packet = *tag;
    ((u8 *)packet)[3] = size;
    *tag = (u32)packet;
    ((u8 *)tag)[3] = 0;
    return packet + (u8)size + 1;
}
#else
INCLUDE_ASM("asm/nonmatchings/syslib/libgs/2d_com1", _make_packet);
#endif
