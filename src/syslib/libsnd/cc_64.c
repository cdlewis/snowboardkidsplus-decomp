#include "common.h"

#include "libsnd_private.h"
extern s32 _SsReadDeltaValue(s16, s16);
extern void _SsVmDamperOff(void), _SsVmDamperOn(void);
void _SsContDamper(s16 id, s16 seq, u8 value) {
    struct SeqStruct *s = &D_8013BF10[id][seq];
    if (value < 64)
        _SsVmDamperOff();
    else
        _SsVmDamperOn();
    s->delta_value = _SsReadDeltaValue(id, seq);
}
