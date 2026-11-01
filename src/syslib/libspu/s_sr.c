#include "common.h"

#ifdef NON_MATCHING
#include "libspu_private.h"
extern int D_80132E18, D_80132E1C, D_80132E20;

/* Best observed instruction-sequence similarity: 42.31% (22/52 words;
 * GCC 2.7.2). */
int SpuSetReverb(int on) {
    u16 cnt;
    switch (on) {
    case 0:
        cnt = _spu_RXX->rxx.spucnt;
        D_80132E18 = 0;
        _spu_RXX->rxx.spucnt = cnt & ~0x80;
        break;
    case 1:
        if (D_80132E1C != 1 && _SpuIsInAllocateArea_(D_80132E20)) {
            cnt = _spu_RXX->rxx.spucnt;
            D_80132E18 = 0;
            _spu_RXX->rxx.spucnt = cnt & ~0x80;
        } else {
            cnt = _spu_RXX->rxx.spucnt;
            D_80132E18 = 1;
            _spu_RXX->rxx.spucnt = cnt | 0x80;
        }
        break;
    }
    return D_80132E18;
}
#else
INCLUDE_ASM("asm/nonmatchings/syslib/libspu/s_sr", SpuSetReverb);
#endif
