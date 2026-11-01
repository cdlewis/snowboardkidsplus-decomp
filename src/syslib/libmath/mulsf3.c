#include "common.h"

#ifdef NON_MATCHING
extern int _err_math(int, int);
/* Best observed instruction-sequence similarity: 35.00% (28/80 words;
 * GCC 2.8.0, assembler reordering). */
float __mulsf3(float a, float b) {
    union { float f; u32 u; } x, y, r;
    u32 sign, m1, m2, prod;
    int exp;
    x.f = a; y.f = b;
    sign = (x.u ^ y.u) & 0x80000000;
    if (!(x.u & 0x7fffffff) || !(y.u & 0x7fffffff)) { r.u = sign; return r.f; }
    exp = ((x.u >> 23) & 0xff) + ((y.u >> 23) & 0xff);
    m1 = (x.u & 0x7fffff) | 0x800000;
    m2 = (y.u & 0x7fffff) | 0x800000;
    prod = (m1 >> 8) * (m2 >> 8);
    prod += ((m1 & 0xff) * (m2 >> 8)) >> 8;
    prod += ((m2 & 0xff) * (m1 >> 8)) >> 8;
    if (prod & 0x80000000) {
        prod = (prod + 0x80) >> 8;
        exp -= 0x7e;
    } else {
        prod += 0x40;
        if (prod & 0x80000000) { prod >>= 8; exp -= 0x7e; }
        else { prod >>= 7; exp -= 0x7f; }
    }
    prod &= 0xff7fffff;
    if (exp >= 0xff) {
        _err_math(0x22, 0x0c);
        r.u = sign ? 0xff800000 : 0x7f800000;
    } else r.u = sign | (exp << 23) | prod;
    return r.f;
}
#else
INCLUDE_ASM("asm/nonmatchings/syslib/libmath/mulsf3", __mulsf3);
#endif
