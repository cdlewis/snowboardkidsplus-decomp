#include "common.h"

INCLUDE_ASM("asm/nonmatchings/syslib/libcd/c_011", StCdInterrupt);

void mem2mem(s32 *dst, s32 *src, u32 num, u32 completion) {
    u32 i;
    for (i = 0; i < num; i++)
        *dst++ = *src++;
}

#ifdef NON_MATCHING
extern volatile u8 *D_80134DB8, *D_80134D9C;
extern volatile u32 *D_80134DB4;
extern const char D_80012A60[];
extern int printf(const char *, ...);
/* Best observed instruction-sequence similarity: 37.74% (40/106 words;
 * GCC 2.8.0). */
void func_800BA430(int channel, u32 addr, u32 blocks, u32 words, u32 control, u8 enable) {
    volatile u32 *dma;
    volatile u32 dummy;
    int i = 0;
    while (*(volatile u32 *)(0x1f801088 + channel * 16) & 0x1000000) {
        if (i == 0x10000) { printf(D_80012A60, *(volatile u32 *)(0x1f801088 + channel * 16)); break; }
        i++;
    }
    if (enable == 1) D_80134DB8[2] |= 1 << channel;
    else D_80134DB8[2] &= ~(1 << channel);
    dummy = *(volatile u32 *)D_80134DB8;
    dma = (volatile u32 *)(0x1f801080 + channel * 16);
    *D_80134DB4 |= 1 << (channel * 4 + 3);
    dma[0] = addr;
    dma[1] = (blocks << 16) | words;
    while (!(*D_80134D9C & 0x40)) { }
    dma[2] = control;
    dummy = dma[2];
}
#else
INCLUDE_ASM("asm/nonmatchings/syslib/libcd/c_011", func_800BA430);
#endif
