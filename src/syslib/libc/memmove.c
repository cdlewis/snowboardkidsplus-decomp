#include "common.h"

#ifdef NON_MATCHING
/* Best observed instruction-sequence similarity: 92.86% (26/28 words;
 * GCC 2.8.0, -fno-builtin). */
void *memmove(void *dest, const void *src, int len) {
    u8 *d = dest;
    const u8 *s = src;

    if (d >= s) {
        while (len-- > 0) d[len] = s[len];
    } else {
        while (len-- > 0) *d++ = *s++;
    }
    return d;
}
#else
INCLUDE_ASM("asm/nonmatchings/syslib/libc/memmove", memmove);
#endif
