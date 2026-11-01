#include "common.h"

#ifdef NON_MATCHING
#include "libsnd_private.h"
extern s32 _SsReadDeltaValue(s16, s16);
extern void _SsVmPitchBend(s16, s8, u8, u8);
/* Best observed instruction-sequence similarity: 79.55% (35/44 words). */
void _SsSetPitchBend(s16 id, s16 seq) {
    struct SeqStruct *s = &D_8013BF10[id][seq];
    _SsVmPitchBend(id | (seq << 8), (s8)s->vab_id,
                   s->programs[s->channel_idx], *s->seq_ptr++);
    s->delta_value = _SsReadDeltaValue(id, seq);
}
#else
INCLUDE_ASM("asm/nonmatchings/syslib/libsnd/midibend", _SsSetPitchBend);
#endif
