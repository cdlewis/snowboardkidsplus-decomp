#include "common.h"

#ifdef NON_MATCHING
#include "libspu_private.h"
extern int _spu_rev_flag, _spu_rev_reserve_wa, _spu_rev_offsetaddr;

/* Best observed instruction-sequence similarity: 42.31% (22/52 words;
 * GCC 2.7.2). */
int SpuSetReverb(int on) {
    u16 cnt;
    switch (on) {
    case 0:
        cnt = _spu_RXX->rxx.spucnt;
        _spu_rev_flag = 0;
        _spu_RXX->rxx.spucnt = cnt & ~0x80;
        break;
    case 1:
        if (_spu_rev_reserve_wa != 1 && _SpuIsInAllocateArea_(_spu_rev_offsetaddr)) {
            cnt = _spu_RXX->rxx.spucnt;
            _spu_rev_flag = 0;
            _spu_RXX->rxx.spucnt = cnt & ~0x80;
        } else {
            cnt = _spu_RXX->rxx.spucnt;
            _spu_rev_flag = 1;
            _spu_RXX->rxx.spucnt = cnt | 0x80;
        }
        break;
    }
    return _spu_rev_flag;
}
#else
INCLUDE_ASM("asm/nonmatchings/syslib/libspu/s_sr", SpuSetReverb);
#endif
