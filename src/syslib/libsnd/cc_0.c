#include "common.h"

#ifdef NON_MATCHING
#include "libsnd_private.h"
extern s32 _SsReadDeltaValue(s16, s16);
/* Best observed instruction-sequence similarity: 81.25% (26/32 words, including padding). */
void _SsContBankChange(s16 id, s16 seq) {
    struct SeqStruct *s = &D_8013BF10[id][seq];
    s->vab_id = *s->seq_ptr++;
    s->delta_value = _SsReadDeltaValue(id, seq);
}
#else
INCLUDE_ASM("asm/nonmatchings/syslib/libsnd/cc_0", _SsContBankChange);
#endif
