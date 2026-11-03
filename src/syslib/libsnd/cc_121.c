#include "common.h"

#include "libsnd_private.h"
extern s32 _SsReadDeltaValue(s16, s16);
extern void SsUtReverbOff(void), _SsVmDamperOff(void);
void _SsContResetAll(s16 id, s16 seq) {
    struct SeqStruct *s = &_ss_score[id][seq];
    SsUtReverbOff();
    _SsVmDamperOff();
    {
        int channel = s->channel_idx;
        s->programs[channel] = channel;
    }
    s->unk18 = 0;
    s->unk19 = 0;
    s->vol[s->channel_idx] = 0x7f;
    s->panpot[s->channel_idx] = 0x40;
    s->delta_value = _SsReadDeltaValue(id, seq);
}
