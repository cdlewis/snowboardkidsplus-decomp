#include "common.h"

#include "libsnd_private.h"
extern s32 _SsReadDeltaValue(s16, s16);
extern void _SsVmSetVol(s16, s8, u8, u16, u8);
void _SsContMainVol(s16 id, s16 seq, u8 value) {
    struct SeqStruct *s = &_ss_score[id][seq];
    int channel = s->channel_idx;
    _SsVmSetVol(id | (seq << 8), s->vab_id, s->programs[channel],
                value, s->panpot[channel]);
    s->vol[channel] = value;
    s->delta_value = _SsReadDeltaValue(id, seq);
}
