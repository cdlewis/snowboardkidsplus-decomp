#include "common.h"

#ifdef NON_MATCHING
#include "libsnd_private.h"
extern s32 _SsReadDeltaValue(s16, s16);
extern void _SsVmDamperOff(void), _SsVmDamperOn(void);
/* Best observed instruction-sequence similarity: 86.36% (38/44 words,
 * including target padding). */
void _SsContDamper(s16 id, s16 seq, u8 value) {
    struct SeqStruct *s = &D_8013BF10[id][seq];
    if (value < 64)
        _SsVmDamperOff();
    else
        _SsVmDamperOn();
    s->delta_value = _SsReadDeltaValue(id, seq);
}
#else
INCLUDE_ASM("asm/nonmatchings/syslib/libsnd/cc_64", _SsContDamper);
#endif
