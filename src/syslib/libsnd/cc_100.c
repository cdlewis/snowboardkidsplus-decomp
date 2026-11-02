#include "common.h"

#include "libsnd_private.h"
extern s32 _SsReadDeltaValue(s16, s16);
void _SsContRpn1(s16 id, s16 seq, u8 value) {
    struct SeqStruct *s = &D_8013BF10[id][seq];
    s->unk18 = value;
    s->unk1E++;
    s->delta_value = _SsReadDeltaValue(id, seq);
}
