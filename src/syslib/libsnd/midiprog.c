#include "common.h"

#ifdef NON_MATCHING
#include "libsnd_private.h"
extern s32 _SsReadDeltaValue(s16, s16);
/* Best observed instruction-sequence similarity: 85.71% (24/28 words, including padding). */
void _SsSetProgramChange(s16 id, s16 seq, u8 value) {
    struct SeqStruct *s = &D_8013BF10[id][seq];
    s->programs[s->channel_idx] = value;
    s->delta_value = _SsReadDeltaValue(id, seq);
}
#else
INCLUDE_ASM("asm/nonmatchings/syslib/libsnd/midiprog", _SsSetProgramChange);
#endif
