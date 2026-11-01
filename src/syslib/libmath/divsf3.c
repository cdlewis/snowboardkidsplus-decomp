#include "common.h"

#ifdef NON_MATCHING
extern int _err_math(int, int);
/* Best observed instruction-sequence similarity: 33.33% (28/84 words;
 * GCC 2.7.2, assembler reordering). */
float __divsf3(float a, float b) {
    union { float f; u32 u; } x, y, r;
    int exp, num, den, bit, q = 0;
    u32 sign;
    x.f = a; y.f = b;
    exp = ((x.u >> 23) & 0xff) - ((y.u >> 23) & 0xff) + 0x7e;
    sign = (x.u ^ y.u) & 0x80000000;
    if (!(y.u & 0x7fffffff)) { r.u = sign ? 0x4f800000 : 0x4f000000; return r.f; }
    if (!(x.u & 0x7fffffff)) { r.u = sign; return r.f; }
    num = (x.u & 0x7fffff) | 0x800000;
    den = (y.u & 0x7fffff) | 0x800000;
    if (num < den) { num <<= 1; exp--; }
    for (bit = 0x1000000; bit; bit >>= 1) {
        if (num >= den) { q |= bit; num -= den; }
        num <<= 1;
    }
    if (exp >= 0) { q = (q + 1) >> 1; exp++; }
    else { q = (q + (1 << (-exp & 31))) >> ((1 - exp) & 31); exp = 0; }
    q &= 0xff7fffff;
    if (exp >= 0xff) {
        _err_math(0x22, 0x0e);
        r.u = sign ? 0xff800000 : 0x7f800000;
    } else r.u = sign | (exp << 23) | q;
    return r.f;
}
#else
INCLUDE_ASM("asm/nonmatchings/syslib/libmath/divsf3", __divsf3);
#endif
