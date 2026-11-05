#include "common.h"

#include "libsnd_private.h"
extern s32 _SsReadDeltaValue(s16, s16);
void _SsContExternal(s16 id, s16 seq, u8 value) {
    struct SeqStruct *s = &_ss_score[id][seq];
    SsUtSetReverbDepth(value, value);
    s->delta_value = _SsReadDeltaValue(id, seq);
}
