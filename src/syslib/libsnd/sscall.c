#include "common.h"

#ifdef NON_MATCHING
#include "libsnd_private.h"
extern int D_8013BF04, D_8013BF08;
extern s16 D_8013C790, D_8013C792;
extern void _SsVmFlush(void), _SsSndPlay(s16, s16), _SsSndCrescendo(s16, s16), _SsSndDecrescendo(s16, s16), _SsSndTempo(s16, s16), _SsSndPause(s16, s16), _SsSndReplay(s16, s16), _SsSndStop(s16, s16);
/* Best observed instruction-sequence similarity: 87.82% (137/156 words;
 * GCC 2.8.0, assembler reordering). */
void SsSeqCalledTbyT(void) {
    int i, j;
    if (D_8013BF04 == 1) return;
    D_8013BF04 = 1;
    _SsVmFlush();
    for (i = 0; i < D_8013C790; i++) {
        if (D_8013BF08 & (1 << i)) {
            for (j = 0; j < D_8013C792; j++) {
                if (D_8013BF10[i][j].flags & 1) {
                    _SsSndPlay(i, j);
                    if (D_8013BF10[i][j].flags & 0x10) _SsSndCrescendo(i, j);
                    if (D_8013BF10[i][j].flags & 0x20) _SsSndDecrescendo(i, j);
                    if (D_8013BF10[i][j].flags & 0x40) _SsSndTempo(i, j);
                    if (D_8013BF10[i][j].flags & 0x80) _SsSndTempo(i, j);
                }
                if (D_8013BF10[i][j].flags & 2) _SsSndPause(i, j);
                if (D_8013BF10[i][j].flags & 8) _SsSndReplay(i, j);
                if (D_8013BF10[i][j].flags & 4) {
                    _SsSndStop(i, j);
                    D_8013BF10[i][j].flags = 0;
                }
            }
        }
    }
    D_8013BF04 = 0;
}
#else
INCLUDE_ASM("asm/nonmatchings/syslib/libsnd/sscall", SsSeqCalledTbyT);
#endif
