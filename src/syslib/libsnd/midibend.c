#include "common.h"
#include "libsnd_private.h"

extern s32 _SsReadDeltaValue(s16, s16);

void _SsSetPitchBend(s16 id, s16 seq) {
    struct SeqStruct *s = &_ss_score[id][seq];
    u8 bend = *s->seq_ptr;
    s->seq_ptr++;
    _SsVmPitchBend(id | (seq << 8), (s8)s->vab_id,
                   *(s->channel_idx + s->programs), bend);
    s->delta_value = _SsReadDeltaValue(id, seq);
}
