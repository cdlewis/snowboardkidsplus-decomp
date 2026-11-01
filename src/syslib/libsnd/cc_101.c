#include "common.h"

#ifdef NON_MATCHING
#include "libsnd_private.h"
extern s32 _SsReadDeltaValue(s16, s16);
/* Best observed instruction-sequence similarity: 85.71% (24/28 words, including padding). */
void _SsContRpn2(s16 id, s16 seq, u8 value) {
    struct SeqStruct *s = &D_8013BF10[id][seq];
    s->unk19 = value;
    s->unk1E++;
    s->delta_value = _SsReadDeltaValue(id, seq);
}
#else
INCLUDE_ASM("asm/nonmatchings/syslib/libsnd/cc_101", _SsContRpn2);
#endif
