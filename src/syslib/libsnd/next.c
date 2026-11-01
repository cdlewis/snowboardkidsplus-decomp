#include "libsnd_private.h"

void _SsSndNextSep(s16 id, s16 sequence) {
    struct SeqStruct *score = &D_8013BF10[id][sequence];

    score->l_count = 1;
    score->unk21 = 0;
    D_8013BF10[id][sequence].flags &= ~0x100;
    D_8013BF10[id][sequence].flags &= ~8;
    D_8013BF10[id][sequence].flags &= ~2;
    D_8013BF10[id][sequence].flags &= ~4;
    D_8013BF10[id][sequence].flags &= ~0x200;
    score->play_mode = 1;
    score->seq_ptr = score->read_pos;
    D_8013BF10[id][sequence].flags |= 1;
}
