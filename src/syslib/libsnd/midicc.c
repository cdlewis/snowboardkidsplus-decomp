#include "common.h"

#ifdef NON_MATCHING
#include "libsnd_private.h"
typedef void (*ControlHandler)(s16, s16, u8);
extern ControlHandler D_8013BE88, D_8013BE8C, D_8013BE90, D_8013BE94, D_8013BE98;
extern ControlHandler D_8013BE9C, D_8013BEA0, D_8013BEA4, D_8013BEA8, D_8013BEAC;
extern void (*D_8013BEB0)(s16, s16);
extern int _SsReadDeltaValue(s16, s16);
/* Best observed instruction-sequence similarity: 54.29% (76/140 words;
 * GCC 2.7.2). */
void _SsSetControlChange(s16 id, s16 seq, u8 control) {
    struct SeqStruct *s = &D_8013BF10[id][seq];
    u8 value = *s->seq_ptr++;
    switch (control) {
    case 0: s->vab_id = value; break;
    case 6: D_8013BE88(id, seq, value); return;
    case 7: D_8013BE8C(id, seq, value); return;
    case 10: D_8013BE90(id, seq, value); return;
    case 11: D_8013BE94(id, seq, value); return;
    case 64: D_8013BE98(id, seq, value); return;
    case 91: D_8013BEAC(id, seq, value); return;
    case 98: D_8013BE9C(id, seq, value); return;
    case 99: D_8013BEA0(id, seq, value); return;
    case 100: D_8013BEA4(id, seq, value); return;
    case 101: D_8013BEA8(id, seq, value); return;
    case 121: D_8013BEB0(id, seq); return;
    }
    s->delta_value = _SsReadDeltaValue(id, seq);
}
#else
INCLUDE_ASM("asm/nonmatchings/syslib/libsnd/midicc", _SsSetControlChange);
#endif
