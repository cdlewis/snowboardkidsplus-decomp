#include "common.h"

#ifdef NON_MATCHING
extern s16 D_80134DEC[];

/* Best observed instruction-sequence similarity: 70.00% (28/40 words;
 * GCC 2.7.2, assembler reordering). */
s32 rcos(s32 a) {
    s32 value;
    if (a < 0) a = -a;
    a &= 0xfff;
    if (a <= 0x800) {
        if (a <= 0x400) value = D_80134DEC[0x400 - a];
        else value = -D_80134DEC[a - 0x400];
    } else {
        if (a <= 0xc00) value = -D_80134DEC[0xc00 - a];
        else value = D_80134DEC[a - 0xc00];
    }
    return value;
}
#else
INCLUDE_ASM("asm/nonmatchings/syslib/libgte/geo_01", rcos);
#endif
