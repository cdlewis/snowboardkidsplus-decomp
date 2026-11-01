#include "common.h"

#ifdef NON_MATCHING
#include "libsnd_private.h"
extern s32 _SsReadDeltaValue(s16, s16);
extern void _SsVmSetVol(s16, s8, u8, u16, u8);
/* Best observed instruction-sequence similarity: 92.31% (48/52 words,
 * including target padding). */
void _SsContMainVol(s16 id, s16 seq, u8 value) {
    struct SeqStruct *s = &D_8013BF10[id][seq];
    int channel = s->channel_idx;
    _SsVmSetVol(id | (seq << 8), s->vab_id, s->programs[channel],
                value, s->panpot[channel]);
    s->vol[channel] = value;
    s->delta_value = _SsReadDeltaValue(id, seq);
}
#else
INCLUDE_ASM("asm/nonmatchings/syslib/libsnd/cc_7", _SsContMainVol);
#endif
