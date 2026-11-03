#include "libsnd_private.h"

extern void (*_SsMarkCallback[][16])(s16, s16, u8);
extern s32 _SsReadDeltaValue(s16, s16);

void _SsContNrpn1(s16 id, s16 seq, u8 value) {
    struct SeqStruct *s = &_ss_score[id][seq];
    if (s->unk1B == 40 && _SsMarkCallback[id][seq])
        _SsMarkCallback[id][seq](id, seq, value);
    if (s->unk1B != 30 && s->unk1B != 20 && s->unk1B != 40) {
        s->fn_idx = value;
        s->unk1C = 0;
        s->unk1F++;
    }
    s->delta_value = _SsReadDeltaValue(id, seq);
}
