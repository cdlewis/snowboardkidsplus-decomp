#include "common.h"

extern s32 sin_1(s32);

s32 rsin(s32 a) {
    s32 value;
    if (a >= 0) value = sin_1(a & 0xfff);
    else value = -sin_1(-a & 0xfff);
    return value;
}

#ifdef NON_MATCHING
extern s16 D_80134DEC[];

/* Best observed instruction-sequence similarity: 81.08% (30/37 words;
 * GCC 2.7.2). */
s32 sin_1(s32 a) {
    if (a <= 0x800) {
        if (a <= 0x400) return D_80134DEC[a];
        return D_80134DEC[0x800 - a];
    }
    if (a > 0xc00) return -D_80134DEC[0x1000 - a];
    return -D_80134DEC[a - 0x800];
}
#else
INCLUDE_ASM("asm/nonmatchings/syslib/libgte/geo_00", sin_1);
#endif
