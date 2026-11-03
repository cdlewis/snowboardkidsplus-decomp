#include "common.h"

#ifdef NON_MATCHING
/* Best observed instruction-sequence similarity: 50.89% (57/112 words; GCC 2.8.0, -mno-split-addresses; target includes padding). */
#include "libspu_private.h"
extern u32 _spu_keystat, _spu_RQmask, _spu_RQvoice, _spu_env;
extern u16 _spu_RQ[];
void SpuSetKey(int on, u32 voices) {
    u32 hi;
    voices &= 0xffffff;
    hi = voices >> 16;
    if (on == 1) {
        if (_spu_env & 1) {
            _spu_RQ[0] = voices; _spu_RQ[1] = hi;
            _spu_RQmask |= 1; _spu_RQvoice |= voices;
            if (_spu_RQ[2] & voices) _spu_RQ[2] &= ~voices;
            if (_spu_RQ[3] & hi) _spu_RQ[3] &= ~hi;
        } else {
            _spu_RXX->raw[0xc4] = voices; _spu_RXX->raw[0xc5] = hi;
            _spu_keystat |= voices;
        }
    } else if (on == 0) {
        if (_spu_env & 1) {
            _spu_RQ[2] = voices; _spu_RQ[3] = hi;
            _spu_RQmask |= 1; _spu_RQvoice &= ~voices;
            if (_spu_RQ[0] & voices) _spu_RQ[0] &= ~voices;
            if (_spu_RQ[1] & hi) _spu_RQ[1] &= ~hi;
        } else {
            _spu_RXX->raw[0xc6] = voices; _spu_RXX->raw[0xc7] = hi;
            _spu_keystat &= ~voices;
        }
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/syslib/libspu/s_sk", SpuSetKey);
#endif
