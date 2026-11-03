#include "common.h"

#ifdef NON_MATCHING
#include "libsnd_private.h"
extern int _snd_ev_flag, _snd_openflag;
extern s16 _snd_seq_s_max, _snd_seq_t_max;
extern void _SsVmFlush(void), _SsSndPlay(s16, s16), _SsSndCrescendo(s16, s16), _SsSndDecrescendo(s16, s16), _SsSndTempo(s16, s16), _SsSndPause(s16, s16), _SsSndReplay(s16, s16), _SsSndStop(s16, s16);
/* Best observed instruction-sequence similarity: 87.82% (137/156 words;
 * GCC 2.8.0, assembler reordering). */
void SsSeqCalledTbyT(void) {
    int i, j;
    if (_snd_ev_flag == 1) return;
    _snd_ev_flag = 1;
    _SsVmFlush();
    for (i = 0; i < _snd_seq_s_max; i++) {
        if (_snd_openflag & (1 << i)) {
            for (j = 0; j < _snd_seq_t_max; j++) {
                if (_ss_score[i][j].flags & 1) {
                    _SsSndPlay(i, j);
                    if (_ss_score[i][j].flags & 0x10) _SsSndCrescendo(i, j);
                    if (_ss_score[i][j].flags & 0x20) _SsSndDecrescendo(i, j);
                    if (_ss_score[i][j].flags & 0x40) _SsSndTempo(i, j);
                    if (_ss_score[i][j].flags & 0x80) _SsSndTempo(i, j);
                }
                if (_ss_score[i][j].flags & 2) _SsSndPause(i, j);
                if (_ss_score[i][j].flags & 8) _SsSndReplay(i, j);
                if (_ss_score[i][j].flags & 4) {
                    _SsSndStop(i, j);
                    _ss_score[i][j].flags = 0;
                }
            }
        }
    }
    _snd_ev_flag = 0;
}
#else
INCLUDE_ASM("asm/nonmatchings/syslib/libsnd/sscall", SsSeqCalledTbyT);
#endif
