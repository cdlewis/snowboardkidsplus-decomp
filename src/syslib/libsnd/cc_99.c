#include "common.h"

#ifdef NON_MATCHING
#include "libsnd_private.h"
extern s32 _SsReadDeltaValue(s16, s16);
/* Best observed instruction-sequence similarity: 73.75% (59/80 words,
 * including target padding). */
void _SsContNrpn2(s16 id, s16 seq, u8 value) {
    struct SeqStruct *s = &_ss_score[id][seq];
    if (value == 20) {
        s->unk1B = value;
        s->unk1C = 1;
        s->delta_value = _SsReadDeltaValue(id, seq);
        s->next_sep_pos = s->seq_ptr;
    } else if (value == 30) {
        s->unk1B = value;
        if (s->unk1D == 0) {
            s->unk15 = 0;
            s->delta_value = _SsReadDeltaValue(id, seq);
        } else if (s->unk1D < 127) {
            s->unk1D--;
            s->delta_value = _SsReadDeltaValue(id, seq);
            if (s->unk1D)
                s->seq_ptr = s->next_sep_pos;
            else
                s->unk15 = 0;
        } else {
            _SsReadDeltaValue(id, seq);
            s->delta_value = 0;
            s->seq_ptr = s->next_sep_pos;
        }
    } else {
        s->unk1B = value;
        s->unk1F++;
        s->delta_value = _SsReadDeltaValue(id, seq);
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/syslib/libsnd/cc_99", _SsContNrpn2);
#endif
