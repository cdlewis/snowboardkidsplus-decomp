#include "common.h"

#ifdef NON_MATCHING
extern u8 D_80139F1C[];
/* Best observed instruction-sequence similarity: 61.67% (37/60 words;
 * GCC 2.8.0, -mno-split-addresses; target includes padding). */
void DecDCTvlcBuild(u16 *table) {
    u8 *out = (u8 *)table, *src = D_80139F1C;
    int distance = 0, count, i;
    do {
        count = *src++;
        if (count < 0xf0) {
            if (distance) {
                for (; count >= 0; count--) { *out = out[-distance]; out++; }
            } else {
                for (; count >= 0; count--) *out++ = *src++;
            }
        } else if (count == 0xf0) {
            distance = 0;
        } else {
            distance = ((count << 8) | *src++) - 0xf0ff;
        }
    } while (distance != 0xf00);
    for (i = 4; i <= 0x87ff; i++) table[i] ^= table[i - 4];
}
#else
INCLUDE_ASM("asm/nonmatchings/syslib/libpress/build", DecDCTvlcBuild);
#endif
