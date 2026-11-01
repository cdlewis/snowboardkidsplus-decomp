#include "common.h"
#include "psx/libetc.h"

#define DMA_DATA __attribute__((section(".data.dma_callbacks"), aligned(4)))

volatile u32 *gDMAInterruptControl DMA_DATA = (volatile u32 *)0x1F8010F4;
DmaCallback dma_cb[8] DMA_DATA = {0};

extern void DMA_memclr(s32 *, int);
extern void trapIntrDMA(void);
extern DmaCallback setIntrDMA(int, DmaCallback);
void *startIntrDMA(void) {
    DMA_memclr((s32 *)dma_cb, 8);
    *gDMAInterruptControl = 0;
    InterruptCallback(3, trapIntrDMA);
    return (void *)setIntrDMA;
}

typedef struct { volatile u32 madr, bcr, chcr, pad; } DmaChannel;
extern DmaChannel *D_80134900;
extern const char D_80012560[], D_8001257C[];
extern int printf(const char *, ...);

void trapIntrDMA(void) {
    u32 flags;
    int i;
    flags = (*gDMAInterruptControl >> 24) & 0x7f;
    while (flags) {
        for (i = 0; flags && i < 7; i++, flags >>= 1) {
            if (flags & 1) {
                *gDMAInterruptControl &= 0xffffff | (1 << (i + 24));
                if (dma_cb[i]) dma_cb[i]();
            }
        }
        flags = (*gDMAInterruptControl >> 24) & 0x7f;
    }
    if ((*gDMAInterruptControl & 0xff000000) == 0x80000000 || (*gDMAInterruptControl & 0x8000)) {
        printf(D_80012560, *gDMAInterruptControl);
        for (i = 0; i < 7; i++) printf(D_8001257C, i, D_80134900[i].madr);
    }
}

#ifdef NON_MATCHING
/* Best observed instruction-sequence similarity: 97.67% (42/43 words;
 * SDK GCC 2.8.0, -mno-split-addresses). */
DmaCallback setIntrDMA(int ch, DmaCallback func) {
    DmaCallback *slot;
    DmaCallback old;

    slot = dma_cb;
    slot += ch;
    old = *slot;

    if (func != old) {
        if (func != 0) {
            volatile u32 *p = gDMAInterruptControl;
            u32 d, bits;
            *slot = func;
            d = *p & 0xFFFFFF;
            bits = (1 << (ch + 0x10)) | 0x800000;
            *p = d | bits;
        } else {
            volatile u32 *p = gDMAInterruptControl;
            *slot = 0;
            *p = (*p & 0xFFFFFF | 0x800000) & ~(1 << (ch + 0x10));
        }
    }
    return old;
}
#else
INCLUDE_ASM("asm/nonmatchings/syslib/libetc/intr_dma", setIntrDMA);
#endif

void DMA_memclr(s32 *p, int n) {
    int i = n - 1;
    if (n != 0) {
        do {
            *p = 0;
            i--;
            p++;
        } while (i != -1);
    }
}
