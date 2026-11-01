#include "common.h"

#ifdef NON_MATCHING
#include "libspu_private.h"
extern void _spu_gcSPU(void);

/* Best observed instruction-sequence similarity: 87.50% (28/32 words,
 * -mno-split-addresses; target includes one padding word). */
void SpuFree(u32 addr) {
    int i;

    for (i = 0; i < _spu_AllocBlockNum; i++) {
        if (_spu_memList[i].addr & 0x40000000) break;
        if (_spu_memList[i].addr == addr) {
            _spu_memList[i].addr |= 0x80000000;
            break;
        }
    }
    _spu_gcSPU();
}
#else
INCLUDE_ASM("asm/nonmatchings/syslib/libspu/s_m_f", SpuFree);
#endif
