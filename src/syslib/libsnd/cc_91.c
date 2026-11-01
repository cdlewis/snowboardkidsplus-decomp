#include "common.h"

#ifdef NON_MATCHING
#include "libsnd_private.h"
extern s32 _SsReadDeltaValue(s16, s16);
extern void SsUtSetReverbDepth(s16, s16);
/* Best observed instruction-sequence similarity: 83.33% (30/36 words,
 * including target padding). */
void _SsContExternal(s16 id, s16 seq, u8 value) {
    struct SeqStruct *s = &D_8013BF10[id][seq];
    SsUtSetReverbDepth(value, value);
    s->delta_value = _SsReadDeltaValue(id, seq);
}
#else
INCLUDE_ASM("asm/nonmatchings/syslib/libsnd/cc_91", _SsContExternal);
#endif
