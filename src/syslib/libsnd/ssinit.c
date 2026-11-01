#include "common.h"

#ifdef NON_MATCHING
extern u16 D_80132DD0[], D_80132DE0[];
extern u32 D_8013BF90[32][16];
extern int D_8013BF0C, D_8013BF08, D_8013BF04;
extern void _SsVmInit(int);
/* Best observed instruction-sequence similarity: 43.33% (26/60 words;
 * target includes padding). */
void _SsInit(void) {
    volatile u16 *spu = (volatile u16 *)0x1f801c00;
    int i, j;
    for (i = 0; i < 24; i++)
        for (j = 0; j < 8; j++) *spu++ = D_80132DD0[j];
    spu = (volatile u16 *)0x1f801d80;
    for (i = 0; i < 16; i++) *spu++ = D_80132DE0[i];
    _SsVmInit(24);
    for (i = 0; i < 32; i++)
        for (j = 15; j >= 0; j--) D_8013BF90[i][j] = 0;
    D_8013BF0C = 60;
    D_8013BF08 = 0;
    D_8013BF04 = 0;
}
#else
INCLUDE_ASM("asm/nonmatchings/syslib/libsnd/ssinit", _SsInit);
#endif
