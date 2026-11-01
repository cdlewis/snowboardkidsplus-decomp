#include "common.h"

#ifdef NON_MATCHING
/* Best observed instruction-sequence similarity: 18.82% (16/85 words;
 * GCC 2.7.2). */
int csqrt_1(int n) {
    volatile int x[8], y[8];
    int i;
    x[0] = n + 0x5d50ad;
    y[0] = n - 0x5d50ad;
    for (i = 1; i < 7; i++) {
        if (i == 4) {
            if (y[i - 1] >= 0) {
                int a = x[i - 1];
                x[i - 1] -= y[i - 1] >> 4;
                y[i - 1] -= a >> 4;
            } else {
                int a = x[i - 1];
                x[i - 1] += y[i - 1] >> 4;
                y[i - 1] += a >> 4;
            }
        }
        if (y[i - 1] >= 0) {
            x[i] = x[i - 1] - (y[i - 1] >> i);
            y[i] = y[i - 1] - (x[i - 1] >> i);
        } else {
            x[i] = x[i - 1] + (y[i - 1] >> i);
            y[i] = y[i - 1] + (x[i - 1] >> i);
        }
    }
    return x[6];
}
#else
INCLUDE_ASM("asm/nonmatchings/syslib/libgte/cor_05", csqrt_1);
#endif

extern int Lzc(int), csqrt_1(int);

int csqrt(int n) {
    int shift, x, y;
    if (!n) return 0;
    x = 8 - Lzc(n);
    if (x >= 0) {
        shift = x >> 1;
        y = n >> (shift * 2);
    } else {
        shift = (x >> 1) + 1;
        y = n << -(shift * 2);
    }
    shift -= 6;
    if (shift < 0) x = csqrt_1(y) >> -shift;
    else x = csqrt_1(y) << shift;
    return x;
}
