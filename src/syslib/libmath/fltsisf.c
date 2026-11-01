#include "common.h"

#ifdef NON_MATCHING
/* Best observed instruction-sequence similarity: 58.33% (28/48 words;
 * GCC 2.7.2, assembler reordering). */
float __floatsisf(int value) {
    union { float f; u32 u; } result;
    s32 exp = 0x9d;
    u32 sign = 0;
    if (value == 0) { result.u = 0; return result.f; }
    if (value < 0) { sign = 0x80000000; value = -value; }
    while (value <= 0xffffff) { value <<= 4; exp -= 4; }
    while (value <= 0x3fffffff) { value <<= 1; exp--; }
    value += 0x40;
    if (value < 0) { exp++; value = (u32)value >> 8; }
    else value = (u32)value >> 7;
    result.u = sign | (exp << 23) | (value & 0xff7fffff);
    return result.f;
}
#else
INCLUDE_ASM("asm/nonmatchings/syslib/libmath/fltsisf", __floatsisf);
#endif
