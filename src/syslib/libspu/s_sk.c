#include "common.h"

#ifdef NON_MATCHING
/* Best observed instruction-sequence similarity: 50.89% (57/112 words; GCC 2.8.0, -mno-split-addresses; target includes padding). */
#include "libspu_private.h"
extern u32 D_80132E10, D_80132E3C, D_80132E38, D_80132E70;
extern u16 D_8013C798[];
void SpuSetKey(int on, u32 voices) {
    u32 hi;
    voices &= 0xffffff;
    hi = voices >> 16;
    if (on == 1) {
        if (D_80132E70 & 1) {
            D_8013C798[0] = voices; D_8013C798[1] = hi;
            D_80132E3C |= 1; D_80132E38 |= voices;
            if (D_8013C798[2] & voices) D_8013C798[2] &= ~voices;
            if (D_8013C798[3] & hi) D_8013C798[3] &= ~hi;
        } else {
            _spu_RXX->raw[0xc4] = voices; _spu_RXX->raw[0xc5] = hi;
            D_80132E10 |= voices;
        }
    } else if (on == 0) {
        if (D_80132E70 & 1) {
            D_8013C798[2] = voices; D_8013C798[3] = hi;
            D_80132E3C |= 1; D_80132E38 &= ~voices;
            if (D_8013C798[0] & voices) D_8013C798[0] &= ~voices;
            if (D_8013C798[1] & hi) D_8013C798[1] &= ~hi;
        } else {
            _spu_RXX->raw[0xc6] = voices; _spu_RXX->raw[0xc7] = hi;
            D_80132E10 &= ~voices;
        }
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/syslib/libspu/s_sk", SpuSetKey);
#endif
