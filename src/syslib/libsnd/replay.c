#include "libsnd_private.h"

void _SsSndReplay(s16 id, s16 sequence) {
    struct SeqStruct *score = &D_8013BF10[id][sequence];

    score->play_mode = 1;
    D_8013BF10[id][sequence].flags &= ~8;
}
