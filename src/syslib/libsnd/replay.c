#include "libsnd_private.h"

void _SsSndReplay(s16 id, s16 sequence) {
    struct SeqStruct *score = &_ss_score[id][sequence];

    score->play_mode = 1;
    _ss_score[id][sequence].flags &= ~8;
}
