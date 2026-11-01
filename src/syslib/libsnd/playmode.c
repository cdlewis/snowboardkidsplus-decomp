#include "common.h"

#ifdef NON_MATCHING
#include "libsnd_private.h"
extern void _SsVmSetSeqVol(s16, u16, u16, int);
/* Best observed instruction-sequence similarity: 83.33% (60/72 words;
 * target includes padding). */
void Snd_SetPlayMode(s16 id, s16 seq, s8 mode, s16 repeat) {
    struct SeqStruct *s = &D_8013BF10[id][seq];
    s->seq_ptr = s->read_pos;
    s->next_sep_pos = s->read_pos;
    s->loop_pos = s->read_pos;
    D_8013BF10[id][seq].flags &= ~0x200;
    D_8013BF10[id][seq].flags &= ~4;
    s->l_count = repeat;
    if (mode == 1) {
        D_8013BF10[id][seq].flags |= 1;
        s->play_mode = 1;
        s->unk21 = 0;
        _SsVmSetSeqVol((s16)(id | (seq << 8)), s->voll, s->volr, 1);
    } else if (mode == 0) {
        D_8013BF10[id][seq].flags |= 2;
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/syslib/libsnd/playmode", Snd_SetPlayMode);
#endif
