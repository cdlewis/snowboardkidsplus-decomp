#include "common.h"

#ifdef NON_MATCHING
#include "libsnd_private.h"
extern s32 _SsReadDeltaValue(s16, s16);
extern void func_800B0604(void), _SsVmDamperOff(void);
/* Best observed instruction-sequence similarity: 87.50% (42/48 words,
 * including target padding). */
void _SsContResetAll(s16 id, s16 seq) {
    struct SeqStruct *s = &D_8013BF10[id][seq];
    func_800B0604();
    _SsVmDamperOff();
    s->programs[s->channel_idx] = s->channel_idx;
    s->unk18 = 0;
    s->unk19 = 0;
    s->vol[s->channel_idx] = 0x7f;
    s->panpot[s->channel_idx] = 0x40;
    s->delta_value = _SsReadDeltaValue(id, seq);
}
#else
INCLUDE_ASM("asm/nonmatchings/syslib/libsnd/cc_121", _SsContResetAll);
#endif
