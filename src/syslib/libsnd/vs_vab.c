#include "common.h"

#ifdef NON_MATCHING
extern u8 D_8013CFE0[];
extern u32 D_8013D040[];
extern u16 D_8013D038;
extern void SpuFree(u32);
/* Best observed instruction-sequence similarity: 81.25% (26/32 words). */
void SsVabClose(s16 id) {
    if (id >= 0 && id < 16 && D_8013CFE0[id] == 1) {
        SpuFree(D_8013D040[id]);
        D_8013CFE0[id] = 0;
        D_8013D038--;
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/syslib/libsnd/vs_vab", SsVabClose);
#endif
