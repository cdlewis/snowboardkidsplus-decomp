#include "common.h"

#ifdef NON_MATCHING
extern int _SN_read(int, int, unsigned, void *);
/* Best observed instruction-sequence similarity: 79.17% (38/48 words;
 * GCC 2.7.2). */
int func_800B5408(int fd, char *buf, unsigned size) {
    int total = 0, result;
    unsigned n;
    while (size) {
        n = size;
        if (size > 0x8000) n = 0x8000;
        result = _SN_read(0, fd, n, buf);
        total += result;
        if (result == -1) return -1;
        buf += result;
        size -= result;
        if (result < (int)n) break;
    }
    return total;
}
#else
INCLUDE_ASM("asm/nonmatchings/syslib/libsn/read", func_800B5408);
#endif
