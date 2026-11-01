#include "libsnd_private.h"

extern void (*D_8013BF90[][16])(s16, s16, u8);
extern s32 _SsReadDeltaValue(s16, s16);

void _SsContNrpn1(s16 id, s16 seq, u8 value) {
    struct SeqStruct *s = &D_8013BF10[id][seq];
    if (s->unk1B == 40 && D_8013BF90[id][seq])
        D_8013BF90[id][seq](id, seq, value);
    if (s->unk1B != 30 && s->unk1B != 20 && s->unk1B != 40) {
        s->fn_idx = value;
        s->unk1C = 0;
        s->unk1F++;
    }
    s->delta_value = _SsReadDeltaValue(id, seq);
}
