#include "libsnd_private.h"

extern void _SsVmSeqKeyOff(s16 id);

void _SsSndPause(s16 id, s16 sequence) {
    struct SeqStruct *score = &_ss_score[id][sequence];

    _SsVmSeqKeyOff(sequence << 8 | id);
    score->play_mode = 0;
    _ss_score[id][sequence].flags &= ~2;
}
