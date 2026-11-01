#include "common.h"

#ifdef NON_MATCHING
extern int _err_math(int, int);

/* Best observed instruction-sequence similarity: 75.00% (36/48 words;
 * GCC 2.7.2, assembler reordering). */
int __fixsfsi(float f) {
    union { float f; s32 i; } v;
    s32 exponent, shift, mantissa;
    v.f = f;
    if (!(v.i & 0x7fffffff)) return v.i & 0x80000000;
    exponent = (v.i >> 23) & 0xff;
    shift = exponent - 0x9d;
    if (shift > 0) {
        if (v.i < 0) return 0x80000000;
        _err_math(0x22, 0x12);
        return 0x7fffffff;
    }
    mantissa = ((v.i & 0x7fffff) | 0x800000) << 7;
    if ((u32)(exponent - 0x7e) >= 32 || !mantissa) return 0;
    mantissa >>= -shift;
    if (v.i < 0) return -mantissa;
    return mantissa;
}
#else
INCLUDE_ASM("asm/nonmatchings/syslib/libmath/fixsfsi", __fixsfsi);
#endif
