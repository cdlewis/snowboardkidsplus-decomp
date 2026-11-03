#include "libsnd_private.h"

void _SsSndNextSep(s16 id, s16 sequence) {
    struct SeqStruct *score = &_ss_score[id][sequence];

    score->l_count = 1;
    score->unk21 = 0;
    _ss_score[id][sequence].flags &= ~0x100;
    _ss_score[id][sequence].flags &= ~8;
    _ss_score[id][sequence].flags &= ~2;
    _ss_score[id][sequence].flags &= ~4;
    _ss_score[id][sequence].flags &= ~0x200;
    score->play_mode = 1;
    score->seq_ptr = score->read_pos;
    _ss_score[id][sequence].flags |= 1;
}
